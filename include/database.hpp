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
    bool insertSessionEntry(int exerciseId, double currentWeight,
                            int set1Reps, int set2Reps, int set3Reps, int set4Reps, int set5Reps,
                            const std::string& sessionEndedAt);
    bool getSettings(int& numSets, int& minReps, int& maxReps, int& pauseSeconds);
    std::vector<int> getExerciseIds();
    struct ExerciseDetails {
        std::string name;
        QByteArray image;
        std::string description;
        double currentWeight = 0.0;
    };
    ExerciseDetails getExerciseDetails(int exerciseId);
    struct LatestSessionData {
        int exerciseId;
        double currentWeight = 0.0;
        std::vector<int> setReps = std::vector<int>(5, -1);
    };
    LatestSessionData getLatestSessionData(int exerciseId);
    sqlite3* getDb() { return db_; }

private:
    sqlite3* db_;
    bool executeQuery(const std::string& query);
};

#endif // DATABASE_HPP