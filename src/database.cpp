#include "database.hpp"
#include <iostream>

Database::Database(const std::string& db_path) : db_(nullptr) {
    int rc = sqlite3_open(db_path.c_str(), &db_);
    if (rc != SQLITE_OK) {
        std::cerr << "Cannot open database: " << sqlite3_errmsg(db_) << std::endl;
        db_ = nullptr;
    }
}

Database::~Database() {
    if (db_) {
        sqlite3_close(db_);
    }
}

bool Database::initialize() {
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
            PauseSeconds     INTEGER DEFAULT 120
        );
    )";
    if (!executeQuery(createSettings)) return false;

    const std::string createSessionLog = R"(
        CREATE TABLE IF NOT EXISTS SessionLog (
            user_id          INTEGER NOT NULL DEFAULT 0,
            exercise_id      INTEGER NOT NULL REFERENCES Exercise(id),
            SessionEndedAt   TEXT NOT NULL,
            WarmupWeight     REAL DEFAULT 0,
            CurrentWeight    REAL NOT NULL,
            Set_1_Reps       INTEGER NOT NULL DEFAULT -1,
            Set_2_Reps       INTEGER NOT NULL DEFAULT -1,
            Set_3_Reps       INTEGER NOT NULL DEFAULT -1,
            Set_4_Reps       INTEGER NOT NULL DEFAULT -1,
            Set_5_Reps       INTEGER NOT NULL DEFAULT -1,
            NextWeight       REAL NOT NULL DEFAULT 0,
            Notes            TEXT,
            PRIMARY KEY (user_id, exercise_id, SessionEndedAt)
        );
    )";
    if (!executeQuery(createSessionLog)) return false;

    // Insert default settings if missing
    executeQuery("INSERT OR IGNORE INTO Settings (user_id) VALUES (0);");

    return true;
}

bool Database::executeQuery(const std::string& query) {
    char* errMsg = nullptr;
    int rc = sqlite3_exec(db_, query.c_str(), nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        std::cerr << "SQL error: " << errMsg << std::endl;
        sqlite3_free(errMsg);
        return false;
    }
    return true;
}

bool Database::hasExercises() {
    sqlite3_stmt* stmt;
    const std::string query = "SELECT COUNT(*) FROM Exercise;";
    if (sqlite3_prepare_v2(db_, query.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db_) << std::endl;
        return false;
    }
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        int count = sqlite3_column_int(stmt, 0);
        sqlite3_finalize(stmt);
        return count > 0;
    }
    sqlite3_finalize(stmt);
    return false;
}

bool Database::hasSettings() {
    sqlite3_stmt* stmt;
    const char* query = "SELECT COUNT(*) FROM Settings WHERE user_id = 0;";
    if (sqlite3_prepare_v2(db_, query, -1, &stmt, nullptr) != SQLITE_OK) return false;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        int count = sqlite3_column_int(stmt, 0);
        sqlite3_finalize(stmt);
        return count > 0;
    }
    sqlite3_finalize(stmt);
    return false;
}

bool Database::insertExercise(const std::string& name, const std::string& description, const void* imageData, int imageSize) {
    sqlite3_stmt* stmt;
    const char* query = "INSERT INTO Exercise (Name, Image, Description) VALUES (?, ?, ?);";
    if (sqlite3_prepare_v2(db_, query, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db_) << std::endl;
        return false;
    }

    sqlite3_bind_text(stmt, 1, name.c_str(), -1, SQLITE_TRANSIENT);
    if (imageData && imageSize > 0) {
        sqlite3_bind_blob(stmt, 2, imageData, imageSize, SQLITE_TRANSIENT);
    } else {
        sqlite3_bind_null(stmt, 2);
    }
    sqlite3_bind_text(stmt, 3, description.c_str(), -1, SQLITE_TRANSIENT);

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        std::cerr << "Failed to save exercise: " << sqlite3_errmsg(db_) << std::endl;
        sqlite3_finalize(stmt);
        return false;
    }
    sqlite3_finalize(stmt);
    return true;
}

bool Database::insertSettings(int numSets, int minReps, int maxReps, int pauseSeconds) {
    sqlite3_stmt* stmt;
    const char* query = "INSERT OR REPLACE INTO Settings (user_id, NumSets, MinReps, MaxReps, PauseSeconds) "
                        "VALUES (0, ?, ?, ?, ?);";
    if (sqlite3_prepare_v2(db_, query, -1, &stmt, nullptr) != SQLITE_OK) return false;

    sqlite3_bind_int(stmt, 1, numSets);
    sqlite3_bind_int(stmt, 2, minReps);
    sqlite3_bind_int(stmt, 3, maxReps);
    sqlite3_bind_int(stmt, 4, pauseSeconds);

    bool ok = (sqlite3_step(stmt) == SQLITE_DONE);
    sqlite3_finalize(stmt);
    return ok;
}

bool Database::insertSessionEntry(int exerciseId, double currentWeight,
                                  int set1, int set2, int set3, int set4, int set5,
                                  const std::string& sessionEndedAt) {
    sqlite3_stmt* stmt;
    const char* query = "INSERT INTO SessionLog (user_id, exercise_id, SessionEndedAt, "
                        "CurrentWeight, Set_1_Reps, Set_2_Reps, Set_3_Reps, Set_4_Reps, Set_5_Reps) "
                        "VALUES (0, ?, ?, ?, ?, ?, ?, ?, ?);";
    if (sqlite3_prepare_v2(db_, query, -1, &stmt, nullptr) != SQLITE_OK) return false;

    sqlite3_bind_int   (stmt, 1, exerciseId);
    sqlite3_bind_text  (stmt, 2, sessionEndedAt.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_double(stmt, 3, currentWeight);
    sqlite3_bind_int   (stmt, 4, set1);
    sqlite3_bind_int   (stmt, 5, set2);
    sqlite3_bind_int   (stmt, 6, set3);
    sqlite3_bind_int   (stmt, 7, set4);
    sqlite3_bind_int   (stmt, 8, set5);

    bool ok = (sqlite3_step(stmt) == SQLITE_DONE);
    sqlite3_finalize(stmt);
    return ok;
}

bool Database::getSettings(int& numSets, int& minReps, int& maxReps, int& pauseSeconds) {
    sqlite3_stmt* stmt;
    const char* query = "SELECT NumSets, MinReps, MaxReps, PauseSeconds FROM Settings WHERE user_id = 0;";
    if (sqlite3_prepare_v2(db_, query, -1, &stmt, nullptr) != SQLITE_OK) return false;

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        numSets      = sqlite3_column_int(stmt, 0);
        minReps      = sqlite3_column_int(stmt, 1);
        maxReps      = sqlite3_column_int(stmt, 2);
        pauseSeconds = sqlite3_column_int(stmt, 3);
        sqlite3_finalize(stmt);
        return true;
    }
    sqlite3_finalize(stmt);
    return false;
}

Database::LatestSessionData Database::getLatestSessionData(int exerciseId) {
    LatestSessionData data;
    data.exerciseId = exerciseId;
    sqlite3_stmt* stmt;
    const char* query = "SELECT CurrentWeight, Set_1_Reps, Set_2_Reps, Set_3_Reps, Set_4_Reps, Set_5_Reps "
                        "FROM SessionLog WHERE user_id = 0 AND exercise_id = ? "
                        "ORDER BY SessionEndedAt DESC LIMIT 1;";
    if (sqlite3_prepare_v2(db_, query, -1, &stmt, nullptr) != SQLITE_OK) return data;

    sqlite3_bind_int(stmt, 1, exerciseId);
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        data.currentWeight = sqlite3_column_double(stmt, 0);
        data.setReps[0] = sqlite3_column_int(stmt, 1);
        data.setReps[1] = sqlite3_column_int(stmt, 2);
        data.setReps[2] = sqlite3_column_int(stmt, 3);
        data.setReps[3] = sqlite3_column_int(stmt, 4);
        data.setReps[4] = sqlite3_column_int(stmt, 5);
    }
    sqlite3_finalize(stmt);
    return data;
}

std::vector<int> Database::getExerciseIds() {
    std::vector<int> ids;
    sqlite3_stmt* stmt;
    const char* query = "SELECT id FROM Exercise ORDER BY id;";
    if (sqlite3_prepare_v2(db_, query, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db_) << std::endl;
        return ids;
    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        ids.push_back(sqlite3_column_int(stmt, 0));
    }

    sqlite3_finalize(stmt);
    return ids;
}

Database::ExerciseDetails Database::getExerciseDetails(int exerciseId) {
    ExerciseDetails details;
    details.currentWeight = 0.0;
    sqlite3_stmt* stmt;
    // Update join to new table (minimal change)
    const char* query = "SELECT e.Name, e.Image, e.Description, sl.CurrentWeight "
                        "FROM Exercise e "
                        "LEFT JOIN SessionLog sl ON e.id = sl.exercise_id "
                        "WHERE e.id = ? "
                        "ORDER BY sl.SessionEndedAt DESC LIMIT 1;";
    if (sqlite3_prepare_v2(db_, query, -1, &stmt, nullptr) != SQLITE_OK) return details;

    sqlite3_bind_int(stmt, 1, exerciseId);
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        details.name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        if (sqlite3_column_type(stmt, 1) != SQLITE_NULL) {
            const void* img = sqlite3_column_blob(stmt, 1);
            int sz = sqlite3_column_bytes(stmt, 1);
            details.image = QByteArray(static_cast<const char*>(img), sz);
        }
        details.description = sqlite3_column_type(stmt, 2) != SQLITE_NULL ?
                              reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2)) : "";
        if (sqlite3_column_type(stmt, 3) != SQLITE_NULL) {
            details.currentWeight = sqlite3_column_double(stmt, 3);
        }
    }
    sqlite3_finalize(stmt);
    return details;
}