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
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            Name TEXT NOT NULL,
            Image BLOB,
            Description TEXT
        );
    )";
    if (!executeQuery(createExercise)) return false;

    const std::string createRoutine = R"(
        CREATE TABLE IF NOT EXISTS Routine (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            NumSets INTEGER NOT NULL,
            MinReps INTEGER NOT NULL,
            MaxReps INTEGER NOT NULL,
            Pause INTEGER NOT NULL
        );
    )";
    if (!executeQuery(createRoutine)) return false;

    const std::string createUserProgress = R"(
        CREATE TABLE IF NOT EXISTS UserProgress (
            Sequence INTEGER NOT NULL,
            Routine_id INTEGER NOT NULL,
            CurrentWeight REAL NOT NULL,
            Progress1 INTEGER NOT NULL,
            Progress2 INTEGER NOT NULL,
            Progress3 INTEGER NOT NULL,
            Progress4 INTEGER NOT NULL,
            Progress5 INTEGER NOT NULL,
            DateTime TEXT NOT NULL
        );
    )";
    return executeQuery(createUserProgress);
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

bool Database::hasRoutines() {
    sqlite3_stmt* stmt;
    const std::string query = "SELECT COUNT(*) FROM Routine;";
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

bool Database::insertRoutine(int numSets, int minReps, int maxReps, int pause) {
    sqlite3_stmt* stmt;
    const char* query = "INSERT INTO Routine (NumSets, MinReps, MaxReps, Pause) VALUES (?, ?, ?, ?);";
    if (sqlite3_prepare_v2(db_, query, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db_) << std::endl;
        return false;
    }

    sqlite3_bind_int(stmt, 1, numSets);
    sqlite3_bind_int(stmt, 2, minReps);
    sqlite3_bind_int(stmt, 3, maxReps);
    sqlite3_bind_int(stmt, 4, pause);

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        std::cerr << "Failed to save routine: " << sqlite3_errmsg(db_) << std::endl;
        sqlite3_finalize(stmt);
        return false;
    }
    sqlite3_finalize(stmt);
    return true;
}

bool Database::insertUserProgress(int sequence, double currentWeight, int progress1, int progress2, int progress3, int progress4, int progress5, const std::string& dateTime) {
    sqlite3_stmt* stmt;
    const char* query = "INSERT INTO UserProgress (Sequence, Routine_id, CurrentWeight, Progress1, Progress2, Progress3, Progress4, Progress5, DateTime) VALUES (?, 1, ?, ?, ?, ?, ?, ?, ?);";
    if (sqlite3_prepare_v2(db_, query, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db_) << std::endl;
        return false;
    }

    sqlite3_bind_int(stmt, 1, sequence);
    sqlite3_bind_double(stmt, 2, currentWeight);
    sqlite3_bind_int(stmt, 3, progress1);
    sqlite3_bind_int(stmt, 4, progress2);
    sqlite3_bind_int(stmt, 5, progress3);
    sqlite3_bind_int(stmt, 6, progress4);
    sqlite3_bind_int(stmt, 7, progress5);
    sqlite3_bind_text(stmt, 8, dateTime.c_str(), -1, SQLITE_TRANSIENT);

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        std::cerr << "Failed to save user progress: " << sqlite3_errmsg(db_) << std::endl;
        sqlite3_finalize(stmt);
        return false;
    }
    sqlite3_finalize(stmt);
    return true;
}

bool Database::getRoutineData(int& numSets, int& minReps, int& maxReps, int& pause) {
    sqlite3_stmt* stmt;
    const char* query = "SELECT NumSets, MinReps, MaxReps, Pause FROM Routine LIMIT 1;";
    if (sqlite3_prepare_v2(db_, query, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db_) << std::endl;
        return false;
    }

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        numSets = sqlite3_column_int(stmt, 0);
        minReps = sqlite3_column_int(stmt, 1);
        maxReps = sqlite3_column_int(stmt, 2);
        pause = sqlite3_column_int(stmt, 3);
        sqlite3_finalize(stmt);
        return true;
    }

    sqlite3_finalize(stmt);
    return false;
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
    details.currentWeight = 0.0; // Default weight

    sqlite3_stmt* stmt;
    const char* query = "SELECT e.Name, e.Image, e.Description, up.CurrentWeight "
                        "FROM Exercise e "
                        "LEFT JOIN UserProgress up ON e.id = up.Sequence "
                        "WHERE e.id = ? "
                        "ORDER BY up.DateTime DESC LIMIT 1;";
    if (sqlite3_prepare_v2(db_, query, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db_) << std::endl;
        return details;
    }

    sqlite3_bind_int(stmt, 1, exerciseId);
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        details.name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        if (sqlite3_column_type(stmt, 1) != SQLITE_NULL) {
            const void* imageData = sqlite3_column_blob(stmt, 1);
            int imageSize = sqlite3_column_bytes(stmt, 1);
            details.image = QByteArray(static_cast<const char*>(imageData), imageSize);
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

Database::UserProgress Database::getUserProgress(int exerciseId) {
    UserProgress progress;
    progress.sequence = exerciseId; // Set sequence
    progress.currentWeight = 0.0;
    progress.progress = std::vector<int>(5, -1); // Default to 5 sets, -1 for unset

    sqlite3_stmt* stmt;
    const char* query = "SELECT CurrentWeight, Progress1, Progress2, Progress3, Progress4, Progress5 "
                        "FROM UserProgress WHERE Sequence = ? ORDER BY DateTime DESC LIMIT 1;";
    if (sqlite3_prepare_v2(db_, query, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db_) << std::endl;
        return progress;
    }

    sqlite3_bind_int(stmt, 1, exerciseId);
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        progress.currentWeight = sqlite3_column_double(stmt, 0);
        for (int i = 0; i < 5; ++i) {
            progress.progress[i] = sqlite3_column_int(stmt, i + 1);
        }
    }

    sqlite3_finalize(stmt);
    return progress;
}