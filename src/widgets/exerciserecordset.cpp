#include "exerciserecordset.hpp"
#include "exerciserecordreps.hpp"
#include "exerciserecord.hpp"
#include <QHBoxLayout>

ExerciseRecordSet::ExerciseRecordSet(int setNum,
                                     int minReps,
                                     int maxReps,
                                     int currentReps,
                                     ExerciseRecord* parent)
    : QWidget(parent),
      setNum_(setNum),
      minReps_(minReps),
      maxReps_(maxReps),
      currentReps_(currentReps),
      parentExercise_(parent)
{
    auto* layout = new QHBoxLayout(this);
    layout->setContentsMargins(4, 4, 4, 4);
    layout->setSpacing(6);

    createRepButtons();
}

void ExerciseRecordSet::createRepButtons()
{
    auto* layout = qobject_cast<QHBoxLayout*>(this->layout());
    if (!layout) return;

    repButtons_.reserve(maxReps_ - minReps_ + 1);

    for (int reps = minReps_; reps <= maxReps_; ++reps)
    {
        bool isHighlighted = (reps == currentReps_);

        auto* button = new ExerciseRecordReps(reps, isHighlighted, this);
        layout->addWidget(button);
        repButtons_.append(button);

        if (isHighlighted)
            currentButton_ = button;
    }

    // Optional: stretch at the end so buttons stay left-aligned
    layout->addStretch(1);
}

void ExerciseRecordSet::setReps(int numReps, ExerciseRecordReps* clickedButton)
{
    if (currentButton_ && currentButton_ != clickedButton)
    {
        currentButton_->Unselect();
    }

    currentReps_ = numReps;
    currentButton_ = clickedButton;

    if (parentExercise_)
    {
        parentExercise_->updateRepsValue(setNum_, numReps);
    }

    emit repsChanged(setNum_, numReps);
}