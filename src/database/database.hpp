#ifndef DATABASE_HPP
#define DATABASE_HPP

#include <sqlite3.h>
#include <string>
#include <vector>
#include <QByteArray>

class Database
{
public:
    Database(const std::string& db_path);
    ~Database();

    enum class DbError {
        Ok,
        DuplicateName,
        Other
    };
    struct WorkoutData {
        struct ExerciseEntry {
            int exerciseId = 0;
            std::string name;
            QByteArray image;
            std::string description;
            double warmupWeight = 0.0;
            double currentWeight = 0.0;
            std::vector<int> setReps = std::vector<int>(5, -1);
            double nextWeight = 0.0;
            // ── New fields (loaded now so future UI changes are easy) ──
            std::string benchNotch;
            std::string muscleGroup;
            std::string notes;
            bool isActive = true;
        };

        std::vector<ExerciseEntry> exercises;
        int numSets = 3;
        int minReps = 8;
        int maxReps = 12;
        int restSeconds = 120;
    };

    bool initialize();
    bool hasExercises();
    bool hasSettings();
    bool insertExercise(const std::string& name, const std::string& description,
                        const void* imageData = nullptr, int imageSize = 0,
                        DbError* outError = nullptr);
    bool insertSettings(int numSets, int minReps, int maxReps, int restSeconds);
    bool insertWorkoutData(const WorkoutData& workoutData, const std::string& workoutTime);
    bool getSettings(int& numSets, int& minReps, int& maxReps, int& restSeconds);
    WorkoutData loadWorkoutData();
    sqlite3* getDb() { return db_; }

private:
    sqlite3* db_;
    bool executeQuery(const std::string& query);
};

#endif