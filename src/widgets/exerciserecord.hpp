#ifndef EXERCISERECORD_HPP
#define EXERCISERECORD_HPP

#include "database/database.hpp"
#include "exerciserecordreps.hpp"
#include <QWidget>
#include <QVector>
#include <vector>
#include <QVBoxLayout>
#include <QValidator>
#include <QLabel>
#include <QTimer>
#include <QLineEdit>

class ExerciseRecordWarmup;
class ExerciseRecordSet;
class WorkoutView;

class ExerciseRecord : public QWidget
{
    Q_OBJECT
public:
    explicit ExerciseRecord(int numSets,
                            int minReps,
                            int maxReps,
                            int restSeconds,
                            const std::vector<int>& currentReps,
                            Database::WorkoutData::ExerciseEntry* entry,
                            bool isWorkoutFinale,
                            WorkoutView* parent = nullptr);

    void enableRecording();

    void startWarmupRest();

    void updateRepsValue(int setNum, int numReps);

    int numberOfSets() const { return numSets_; }
    int minReps() const { return minReps_; }
    int maxReps() const { return maxReps_; }

    std::vector<int> currentReps() const;

signals:
    void finalRestStarted(Database::WorkoutData::ExerciseEntry* entry);
    void exerciseCompleted(Database::WorkoutData::ExerciseEntry* entry);

private:
    void createWarmup();
    void createSets();
    void createRestTimerAndLabel();
    void createWeightWidgets();
    void enableSet(int setIndex);
    void disableSet(int setIndex);

    int numSets_ = 0;
    int minReps_;
    int maxReps_;
    int restSeconds_;
    std::vector<int> currentReps_;
    Database::WorkoutData::ExerciseEntry* entry_ = nullptr;
    bool isWorkoutFinale_ = false;
    WorkoutView* parentWorkout_ = nullptr;

    QVBoxLayout* mainLayout_ = nullptr;
    QDoubleValidator* validator_ = nullptr;

    ExerciseRecordWarmup* warmupWidget_ = nullptr;
    bool hasWarmedUp_ = false;
    bool isWarmupRest_ = false;

    QVector<ExerciseRecordSet*> sets_;
    int currentSetIndex_ = 0;

    void startARestTimerSecond();
    void startRestTimer();
    void onRestTimerTick();
    void RestHasEnded();
    int timerRemainingSeconds_ = 0;
    QString selectRepsText_ = "Select Reps for current set";
    QString restPrefix_ = "";
    QLabel* restLabel_ = nullptr;
    QTimer* restTimer_ = nullptr;
    bool timerIsRunning_ = false;

    QLabel* currentWeightLabel_ = nullptr;
    QLineEdit* currentWeightEdit_ = nullptr;
    QLabel* nextWeightLabel_ = nullptr;
    QLineEdit* nextWeightEdit_ = nullptr;
};

#endif