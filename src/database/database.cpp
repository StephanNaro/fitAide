#include "database.hpp"
#include "utils/errorhelper.hpp"
#include <iostream>
#include <QDateTime>

Database::Database(const std::string& db_path) : db_(nullptr)
{
    int rc = sqlite3_open(db_path.c_str(), &db_);
    if (rc != SQLITE_OK)
    {
        QString err = QString::fromUtf8(sqlite3_errmsg(db_));
        ErrorHelper::showDbError(nullptr, "open database", err);
        db_ = nullptr;
        return;
    }

    // IMPORTANT: Enable foreign key constraint enforcement
    rc = sqlite3_exec(db_, "PRAGMA foreign_keys = ON;", nullptr, nullptr, nullptr);
    if (rc != SQLITE_OK)
    {
        std::cerr << "Failed to enable foreign keys: " << sqlite3_errmsg(db_) << std::endl;
        // Continue anyway — but log it
    }
}

Database::~Database()
{
    if (db_)
    {
        sqlite3_close(db_);
    }
}

bool Database::executeQuery(const std::string& query)
{
    char* errMsg = nullptr;
    int rc = sqlite3_exec(db_, query.c_str(), nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK)
    {
        QString msg = QString::fromUtf8(errMsg ? errMsg : "Unknown error");
        sqlite3_free(errMsg);
        ErrorHelper::showDbError(nullptr, "execute schema query", msg);
        return false;
    }
    return true;
}

bool Database::initialize()
{
    if (!db_) return false;

    const std::string createExercise = R"(
        CREATE TABLE IF NOT EXISTS Exercise (
            id               INTEGER PRIMARY KEY AUTOINCREMENT,
            Name             TEXT NOT NULL UNIQUE,
            Image            BLOB,
            Description      TEXT,
            BenchNotch       TEXT,
            MuscleGroup      TEXT,
            IsActive         INTEGER NOT NULL DEFAULT 1 CHECK (IsActive IN (0,1))
        );
    )";
    if (!executeQuery(createExercise)) return false;

    const std::string createSettings = R"(
        CREATE TABLE IF NOT EXISTS Settings (
            user_id          INTEGER PRIMARY KEY,
            NumSets          INTEGER DEFAULT 3,
            MinReps          INTEGER DEFAULT 8,
            MaxReps          INTEGER DEFAULT 12,
            RestSeconds      INTEGER DEFAULT 120
        );
    )";
    if (!executeQuery(createSettings)) return false;

    const std::string createWorkoutLog = R"(
        CREATE TABLE IF NOT EXISTS WorkoutLog (
            user_id          INTEGER NOT NULL DEFAULT 0,
            exercise_id      INTEGER NOT NULL REFERENCES Exercise(id),
            WorkoutEndedAt   TEXT NOT NULL,
            CurrentWeight    REAL NOT NULL,
            Set_1_Reps       INTEGER NOT NULL DEFAULT -1,
            Set_2_Reps       INTEGER NOT NULL DEFAULT -1,
            Set_3_Reps       INTEGER NOT NULL DEFAULT -1,
            Set_4_Reps       INTEGER NOT NULL DEFAULT -1,
            Set_5_Reps       INTEGER NOT NULL DEFAULT -1,
            NextWeight       REAL NOT NULL DEFAULT 0,
            Notes            TEXT,
            PRIMARY KEY (user_id, exercise_id, WorkoutEndedAt)
        );
    )";
    if (!executeQuery(createWorkoutLog)) return false;

    // Ensure foreign keys are enforced for this connection
    if (sqlite3_exec(db_, "PRAGMA foreign_keys = ON;", nullptr, nullptr, nullptr) != SQLITE_OK)
    {
        std::cerr << "Warning: Could not enable foreign key constraints" << std::endl;
    }

    return true;
}

bool Database::hasSettings()
{
    sqlite3_stmt* stmt;
    const char* query = "SELECT COUNT(*) FROM Settings WHERE user_id = 0;";
    if (sqlite3_prepare_v2(db_, query, -1, &stmt, nullptr) != SQLITE_OK) return false;
    if (sqlite3_step(stmt) == SQLITE_ROW)
    {
        int count = sqlite3_column_int(stmt, 0);
        sqlite3_finalize(stmt);
        return count > 0;
    }
    sqlite3_finalize(stmt);
    return false;
}

bool Database::hasExercises()
{
    sqlite3_stmt* stmt;
    const std::string query = "SELECT COUNT(*) FROM Exercise;";
    if (sqlite3_prepare_v2(db_, query.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
    {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db_) << std::endl;
        return false;
    }
    if (sqlite3_step(stmt) == SQLITE_ROW)
    {
        int count = sqlite3_column_int(stmt, 0);
        sqlite3_finalize(stmt);
        return count > 0;
    }
    sqlite3_finalize(stmt);
    return false;
}

bool Database::insertSettings(int numSets, int minReps, int maxReps, int restSeconds)
{
    sqlite3_stmt* stmt;
    const char* query = "INSERT OR REPLACE INTO Settings (user_id, NumSets, MinReps, MaxReps, RestSeconds) "
                        "VALUES (0, ?, ?, ?, ?);";
    if (sqlite3_prepare_v2(db_, query, -1, &stmt, nullptr) != SQLITE_OK) return false;

    sqlite3_bind_int(stmt, 1, numSets);
    sqlite3_bind_int(stmt, 2, minReps);
    sqlite3_bind_int(stmt, 3, maxReps);
    sqlite3_bind_int(stmt, 4, restSeconds);

    bool ok = (sqlite3_step(stmt) == SQLITE_DONE);
    sqlite3_finalize(stmt);
    return ok;
}

bool Database::insertExercise(const std::string& name, const std::string& description,
                              const void* imageData, int imageSize, DbError* outError)
{
    if (outError) *outError = DbError::Ok;

    sqlite3_stmt* stmt = nullptr;
    const char* query = "INSERT INTO Exercise (Name, Image, Description) VALUES (?, ?, ?);";

    if (sqlite3_prepare_v2(db_, query, -1, &stmt, nullptr) != SQLITE_OK)
    {
        ErrorHelper::showDbError(nullptr, "save exercise", QString::fromUtf8(sqlite3_errmsg(db_)));
        if (outError) *outError = DbError::Other;
        return false;
    }

    sqlite3_bind_text(stmt, 1, name.c_str(), -1, SQLITE_TRANSIENT);
    if (imageData && imageSize > 0)
        sqlite3_bind_blob(stmt, 2, imageData, imageSize, SQLITE_TRANSIENT);
    else
        sqlite3_bind_null(stmt, 2);

    sqlite3_bind_text(stmt, 3, description.c_str(), -1, SQLITE_TRANSIENT);

    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    if (rc == SQLITE_DONE)
        return true;

    // Handle specific errors
    const char* errMsg = sqlite3_errmsg(db_);
    if (strstr(errMsg, "UNIQUE constraint failed") ||
        strstr(errMsg, "constraint failed: Exercise.Name"))
    {
        if (outError) *outError = DbError::DuplicateName;
    }
    else
    {
        std::cerr << "Failed to save exercise: " << errMsg << std::endl;
        if (outError) *outError = DbError::Other;
    }

    return false;
}

bool Database::insertWorkoutData(const WorkoutData& workoutData, const std::string& workoutTime)
{
    if (workoutData.exercises.empty()) return true;

    // Begin transaction
    if (sqlite3_exec(db_, "BEGIN TRANSACTION;", nullptr, nullptr, nullptr) != SQLITE_OK)
    {
        std::cerr << "Failed to begin transaction: " << sqlite3_errmsg(db_) << std::endl;
        return false;
    }

    sqlite3_stmt* stmt = nullptr;
    const char* query = R"(
        INSERT INTO WorkoutLog
            (user_id, exercise_id, WorkoutEndedAt,
             CurrentWeight,
             Set_1_Reps, Set_2_Reps, Set_3_Reps, Set_4_Reps, Set_5_Reps,
             NextWeight, Notes)
        VALUES (0, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?);
    )";

    bool prepareOk = (sqlite3_prepare_v2(db_, query, -1, &stmt, nullptr) == SQLITE_OK);
    if (!prepareOk)
    {
        ErrorHelper::showDbError(nullptr, "prepare workout insert", QString::fromUtf8(sqlite3_errmsg(db_)));
        sqlite3_exec(db_, "ROLLBACK;", nullptr, nullptr, nullptr);
        return false;
    }

    bool allOk = true;
    for (const auto& ex : workoutData.exercises)
    {
        int reps[5] = {-1, -1, -1, -1, -1};
        for (size_t i = 0; i < ex.setReps.size() && i < 5; ++i)
            reps[i] = ex.setReps[i];

        sqlite3_bind_int   (stmt, 1, ex.exerciseId);
        sqlite3_bind_text  (stmt, 2, workoutTime.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_double(stmt, 3, ex.currentWeight);
        sqlite3_bind_int   (stmt, 4, reps[0]);
        sqlite3_bind_int   (stmt, 5, reps[1]);
        sqlite3_bind_int   (stmt, 6, reps[2]);
        sqlite3_bind_int   (stmt, 7, reps[3]);
        sqlite3_bind_int   (stmt, 8, reps[4]);
        sqlite3_bind_double(stmt, 9, ex.nextWeight);
        sqlite3_bind_text  (stmt, 10, ex.notes.c_str(), -1, SQLITE_TRANSIENT);

        if (sqlite3_step(stmt) != SQLITE_DONE)
        {
            ErrorHelper::showDbError(nullptr, QString("insert exercise %1").arg(ex.exerciseId),
                         QString::fromUtf8(sqlite3_errmsg(db_)));
            allOk = false;
            break;                    // Stop on first error
        }

        sqlite3_reset(stmt);          // Reset for next iteration
    }

    sqlite3_finalize(stmt);

    // Commit or rollback
    if (allOk)
    {
        if (sqlite3_exec(db_, "COMMIT;", nullptr, nullptr, nullptr) != SQLITE_OK)
        {
            ErrorHelper::showDbError(nullptr, QString("commit transaction"),
                         QString::fromUtf8(sqlite3_errmsg(db_)));
            allOk = false;
        }
    }
    else
    {
        sqlite3_exec(db_, "ROLLBACK;", nullptr, nullptr, nullptr);
    }

    return allOk;
}

bool Database::getSettings(int& numSets, int& minReps, int& maxReps, int& restSeconds)
{
    sqlite3_stmt* stmt;
    const char* query = "SELECT NumSets, MinReps, MaxReps, RestSeconds FROM Settings WHERE user_id = 0;";
    if (sqlite3_prepare_v2(db_, query, -1, &stmt, nullptr) != SQLITE_OK) return false;

    if (sqlite3_step(stmt) == SQLITE_ROW)
    {
        numSets      = sqlite3_column_int(stmt, 0);
        minReps      = sqlite3_column_int(stmt, 1);
        maxReps      = sqlite3_column_int(stmt, 2);
        restSeconds  = sqlite3_column_int(stmt, 3);
        sqlite3_finalize(stmt);
        return true;
    }
    sqlite3_finalize(stmt);
    return false;
}

QString Database::getLastWorkoutTime() const
{
    if (!db_) return QString();

    sqlite3_stmt* stmt = nullptr;
    const char* q = "SELECT MAX(WorkoutEndedAt) FROM WorkoutLog WHERE user_id = 0;";

    if (sqlite3_prepare_v2(db_, q, -1, &stmt, nullptr) != SQLITE_OK)
        return QString();

    QString lastTime;
    if (sqlite3_step(stmt) == SQLITE_ROW)
    {
        const unsigned char* text = sqlite3_column_text(stmt, 0);
        if (text)
            lastTime = QString::fromUtf8(reinterpret_cast<const char*>(text));
    }
    sqlite3_finalize(stmt);
    return lastTime;
}

bool Database::isCooldownActive() const
{
    QString lastTimeStr = getLastWorkoutTime();
    if (lastTimeStr.isEmpty())
        return false;   // no previous workout → no cooldown

    QDateTime lastTime = QDateTime::fromString(lastTimeStr, "yyyy-MM-dd HH:mm:ss");
    if (!lastTime.isValid())
        return false;

    QDateTime now = QDateTime::currentDateTime();
    qint64 hoursDiff = lastTime.secsTo(now) / 3600;

    return hoursDiff < 48;
}

Database::WorkoutData Database::loadWorkoutData()
{
    WorkoutData workout;

    // 1. Load settings
    {
        sqlite3_stmt* stmt = nullptr;
        const char* q = "SELECT NumSets, MinReps, MaxReps, RestSeconds FROM Settings WHERE user_id = 0;";
        if (sqlite3_prepare_v2(db_, q, -1, &stmt, nullptr) == SQLITE_OK)
        {
            if (sqlite3_step(stmt) == SQLITE_ROW)
            {
                workout.numSets      = sqlite3_column_int(stmt, 0);
                workout.minReps      = sqlite3_column_int(stmt, 1);
                workout.maxReps      = sqlite3_column_int(stmt, 2);
                workout.restSeconds  = sqlite3_column_int(stmt, 3);
            }
            sqlite3_finalize(stmt);
        }
    }

    // 2. Load ALL active exercises + their MOST RECENT data (if any) — ONE query
    {
        sqlite3_stmt* stmt = nullptr;
        const char* q = R"(
            SELECT
                e.id,
                e.Name,
                e.Image,
                e.Description,
                e.BenchNotch,
                e.MuscleGroup,
                e.IsActive,
                COALESCE(sl.CurrentWeight, 0.0),
                COALESCE(sl.Set_1_Reps, -1),
                COALESCE(sl.Set_2_Reps, -1),
                COALESCE(sl.Set_3_Reps, -1),
                COALESCE(sl.Set_4_Reps, -1),
                COALESCE(sl.Set_5_Reps, -1),
                COALESCE(sl.NextWeight, 0.0),
                COALESCE(sl.Notes, '')
            FROM Exercise e
            LEFT JOIN WorkoutLog sl
                ON sl.exercise_id = e.id
               AND sl.user_id = 0
               AND sl.WorkoutEndedAt = (
                    SELECT MAX(WorkoutEndedAt)
                    FROM WorkoutLog
                    WHERE exercise_id = e.id AND user_id = 0
               )
            WHERE e.IsActive = 1
            ORDER BY e.id;
        )";

        if (sqlite3_prepare_v2(db_, q, -1, &stmt, nullptr) == SQLITE_OK)
        {
            while (sqlite3_step(stmt) == SQLITE_ROW)
            {
                WorkoutData::ExerciseEntry ex;
                ex.exerciseId    = sqlite3_column_int(stmt, 0);
                ex.name          = sqlite3_column_text(stmt, 1)
                                   ? reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1))
                                   : "";
                if (sqlite3_column_type(stmt, 2) != SQLITE_NULL)
                {
                    const void* data = sqlite3_column_blob(stmt, 2);
                    int size = sqlite3_column_bytes(stmt, 2);
                    ex.image = QByteArray(static_cast<const char*>(data), size);
                }
                ex.description   = sqlite3_column_text(stmt, 3)
                                   ? reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3))
                                   : "";
                ex.benchNotch    = sqlite3_column_text(stmt, 4)
                                   ? reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4))
                                   : "";
                ex.muscleGroup   = sqlite3_column_text(stmt, 5)
                                   ? reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5))
                                   : "";
                ex.isActive      = sqlite3_column_int(stmt, 6) != 0;
                ex.currentWeight = sqlite3_column_double(stmt, 7);

                ex.setReps = {
                    sqlite3_column_int(stmt, 8),
                    sqlite3_column_int(stmt, 9),
                    sqlite3_column_int(stmt, 10),
                    sqlite3_column_int(stmt, 11),
                    sqlite3_column_int(stmt, 12)
                };

                ex.nextWeight    = sqlite3_column_double(stmt, 13);
                ex.notes         = sqlite3_column_text(stmt, 14)
                                   ? reinterpret_cast<const char*>(sqlite3_column_text(stmt, 14))
                                   : "";

                workout.exercises.push_back(std::move(ex));
            }
            sqlite3_finalize(stmt);
        }
    }
    if (workout.exercises.empty()) return workout; // safety

    // Carry-over: NextWeight from previous workout becomes CurrentWeight
    for (auto& ex : workout.exercises)
    {
        if (ex.nextWeight > 0.0)          // only if we actually set a next weight before
            ex.currentWeight = ex.nextWeight;
        ex.nextWeight = ex.currentWeight;
    }

    return workout;
}