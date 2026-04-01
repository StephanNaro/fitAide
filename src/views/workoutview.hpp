#ifndef WORKOUTVIEW_HPP
#define WORKOUTVIEW_HPP

#include "database/database.hpp"
#include <QMainWindow>
#include <QVBoxLayout>
#include <vector>

class ExerciseStatus;
class ExerciseHead;
class ExerciseInstruct;
class ExerciseRecord;

class WorkoutView : public QMainWindow
{
    Q_OBJECT
public:
    explicit WorkoutView(Database& db, QWidget* parent = nullptr);
    ~WorkoutView() = default;

private slots:
    void onExerciseFinalRestStarted(Database::WorkoutData::ExerciseEntry* entry);
    void onExerciseCompleted(Database::WorkoutData::ExerciseEntry* entry);
    void menuAddExercise();

private:
    void createMenuAndInitialLayout();
    void createExerciseWidgets(int index,
                               ExerciseHead*& head,
                               ExerciseInstruct*& instruct,
                               ExerciseRecord*& record);

    Database& db_;
    Database::WorkoutData workoutData_;
    int numExercises_ = 0;
    int currentExerciseIndex_ = 0;

    bool workoutInProgress_ = false;

    QVBoxLayout* mainLayout_ = nullptr;

    ExerciseHead* currentHead_ = nullptr;
    ExerciseInstruct* currentInstruct_ = nullptr;
    ExerciseRecord* currentRecord_ = nullptr;

    ExerciseStatus* upcomingStatus_ = nullptr;
    ExerciseHead* upcomingHead_ = nullptr;
    ExerciseInstruct* upcomingInstruct_ = nullptr;
    ExerciseRecord* upcomingRecord_ = nullptr;
};

#endif