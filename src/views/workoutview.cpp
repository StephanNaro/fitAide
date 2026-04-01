#include "workoutview.hpp"
#include "widgets/exercisestatus.hpp"
#include "widgets/exercisehead.hpp"
#include "widgets/exerciseinstruct.hpp"
#include "widgets/exerciserecord.hpp"
#include "dialogs/exercisedialog.hpp"
#include <QMessageBox>
#include <QMenuBar>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QValidator>
#include <QTimer>
#include <QDateTime>

WorkoutView::WorkoutView(Database& db, QWidget* parent)
    : QMainWindow(parent), db_(db), currentExerciseIndex_(0),
      workoutInProgress_(false)
{
    setWindowTitle("fitAide");
    setMinimumWidth(400);

    workoutData_ = db_.loadWorkoutData();
    if (workoutData_.exercises.empty()) {
        QMessageBox::critical(this, "Error", "No active exercises found");
        close();
        return;
    }
    numExercises_ = workoutData_.exercises.size();

    createMenuAndInitialLayout();
}

void WorkoutView::createMenuAndInitialLayout() {
    QMenuBar* menuBar = new QMenuBar(this);
    setMenuBar(menuBar);
    QMenu* fileMenu = menuBar->addMenu("File");
    QAction* addExerciseAction = fileMenu->addAction("Add Exercise");
    connect(addExerciseAction, &QAction::triggered, this, &WorkoutView::menuAddExercise);

    QWidget* central = new QWidget(this);
    setCentralWidget(central);

    mainLayout_ = new QVBoxLayout(central);
    mainLayout_->setAlignment(Qt::AlignTop);

    if (workoutData_.exercises.empty()) return;

    currentExerciseIndex_ = 0;
    createExerciseWidgets(0, currentHead_, currentInstruct_, currentRecord_);
    if (currentRecord_)
        currentRecord_->enableRecording();

    mainLayout_->addWidget(currentHead_);
    mainLayout_->addWidget(currentInstruct_);
    mainLayout_->addWidget(currentRecord_);

    mainLayout_->addStretch(1);
}

void WorkoutView::createExerciseWidgets(int index,
                                        ExerciseHead*& headOut,
                                        ExerciseInstruct*& instructOut,
                                        ExerciseRecord*& recordOut)
{
    if (index < 0 || index >= numExercises_) {
        headOut = nullptr;
        instructOut = nullptr;
        recordOut = nullptr;
        return;
    }

    Database::WorkoutData::ExerciseEntry* entry = &workoutData_.exercises[index];

    bool isWorkoutFinale_ = (index >= numExercises_ - 1);

    headOut = new ExerciseHead(QString::fromStdString(entry->name));
    instructOut = new ExerciseInstruct(QString::fromStdString(entry->description), entry->image);
    recordOut = new ExerciseRecord(workoutData_.numSets, workoutData_.minReps, workoutData_.maxReps,
                                   workoutData_.pauseSeconds, entry->setReps, entry, isWorkoutFinale_, this);

    connect(recordOut, &ExerciseRecord::finalRestStarted,
            this, &WorkoutView::onExerciseFinalRestStarted);
    connect(recordOut, &ExerciseRecord::exerciseCompleted,
            this, &WorkoutView::onExerciseCompleted);
}

void WorkoutView::onExerciseFinalRestStarted(Database::WorkoutData::ExerciseEntry* entry)
{
    if (entry != &workoutData_.exercises[currentExerciseIndex_]) return;

    workoutInProgress_ = true;

    // 1. Remove the current instruction (we'll show the next one instead)
    if (currentInstruct_)
    {
        mainLayout_->removeWidget(currentInstruct_);
        currentInstruct_->deleteLater();
        currentInstruct_ = nullptr;
    }

    int nextExercise = currentExerciseIndex_ + 1;
    if (nextExercise >= numExercises_) return;

    // 2. Prepare the upcoming exercise widgets
    upcomingStatus_ = new ExerciseStatus();
    createExerciseWidgets(nextExercise, upcomingHead_, upcomingInstruct_, upcomingRecord_);

    if (!upcomingRecord_) return;

    // 3. Add the preview widgets (this is your nice "preview during final rest" feature)
    if (upcomingStatus_)   mainLayout_->addWidget(upcomingStatus_);
    if (upcomingHead_)     mainLayout_->addWidget(upcomingHead_);
    if (upcomingInstruct_) mainLayout_->addWidget(upcomingInstruct_);
    if (upcomingRecord_)   mainLayout_->addWidget(upcomingRecord_);
}

void WorkoutView::onExerciseCompleted(Database::WorkoutData::ExerciseEntry* entry)
{
    if (entry != &workoutData_.exercises[currentExerciseIndex_]) return;

    // 1. Clean up the "Upcoming..." status label (it was only for preview)
    if (upcomingStatus_)
    {
        mainLayout_->removeWidget(upcomingStatus_);
        upcomingStatus_->deleteLater();
        upcomingStatus_ = nullptr;
    }

    // 2. Remove the old current widgets that we no longer need
    if (currentHead_)
    {
        mainLayout_->removeWidget(currentHead_);
        currentHead_->deleteLater();
        currentHead_ = nullptr;
    }

    if (currentRecord_)
    {
        mainLayout_->removeWidget(currentRecord_);
        currentRecord_->deleteLater();
        currentRecord_ = nullptr;
    }

    // 3. Promote the upcoming widgets to current (they are ALREADY in the layout)
    currentHead_     = upcomingHead_;
    upcomingHead_    = nullptr;

    currentInstruct_ = upcomingInstruct_;
    upcomingInstruct_ = nullptr;

    currentRecord_   = upcomingRecord_;
    upcomingRecord_  = nullptr;

    // 4. Final exercise handling
    if (currentExerciseIndex_ >= numExercises_ - 1)
    {
        QString workoutTime = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
        if (!db_.insertWorkoutData(workoutData_, workoutTime.toStdString()))
        {
            QMessageBox::critical(this, "Error", "Failed to save workout data");
        }
        close();
        return;
    }

    // 5. Move to next exercise
    currentExerciseIndex_++;
    if (currentRecord_)
        currentRecord_->enableRecording();
}

void WorkoutView::menuAddExercise() {
    ExerciseDialog dialog(db_, this);
    if (dialog.exec() == QDialog::Accepted &&
        !workoutInProgress_) {
        // Reload data so new exercises appear immediately
        workoutData_ = db_.loadWorkoutData();
    }
}