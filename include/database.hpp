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
    bool hasSettings();
    bool insertExercise(const std::string& name, const std::string& description,
                        const void* imageData = nullptr, int imageSize = 0);
    bool insertSettings(int numSets, int minReps, int maxReps, int pauseSeconds);
    bool insertWorkoutEntry(int exerciseId, double currentWeight,
                            int set1Reps, int set2Reps, int set3Reps, int set4Reps, int set5Reps,
                            const std::string& workoutEndedAt);
    bool getSettings(int& numSets, int& minReps, int& maxReps, int& pauseSeconds);
    struct WorkoutData {
        struct ExerciseEntry {
            int exerciseId = 0;
            std::string name;
            QByteArray image;
            std::string description;
            double currentWeight = 0.0;
            std::vector<int> setReps = std::vector<int>(5, -1);
        };

        std::vector<ExerciseEntry> exercises;
        int numSets = 3;
        int minReps = 8;
        int maxReps = 12;
        int pauseSeconds = 120;
    };
    WorkoutData loadFullWorkoutData();
    sqlite3* getDb() { return db_; }

private:
    sqlite3* db_;
    bool executeQuery(const std::string& query);
};

#endif // DATABASE_HPP