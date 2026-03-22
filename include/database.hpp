#ifndef DATABASE_HPP
#define DATABASE_HPP

#include <sqlite3.h>
#include <string>
#include <vector>
#include <QByteArray>

class Database {
public:
    Database(const std::string& db_path);
    ~Database();

    bool initialize();
    bool hasExercises();
    bool hasRoutines();
    bool insertExercise(const std::string& name, const std::string& description, const void* imageData, int imageSize);
    bool insertRoutine(int numSets, int minReps, int maxReps, int pause);
    bool insertUserProgress(int sequence, double currentWeight, int progress1, int progress2, int progress3, int progress4, int progress5, const std::string& dateTime);
    bool getRoutineData(int& numSets, int& minReps, int& maxReps, int& pause);
    std::vector<int> getExerciseIds();

    // Struct for exercise details and weight
    struct ExerciseDetails {
        std::string name;
        QByteArray image;
        std::string description;
        double currentWeight;
    };
    ExerciseDetails getExerciseDetails(int exerciseId);

    struct UserProgress {
        int sequence; // Added sequence
        double currentWeight;
        std::vector<int> progress;
    };
    UserProgress getUserProgress(int exerciseId);

    sqlite3* getDb() { return db_; }

private:
    sqlite3* db_;
    bool executeQuery(const std::string& query);
};

#endif // DATABASE_HPP