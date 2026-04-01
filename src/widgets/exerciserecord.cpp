#include "exerciserecordset.hpp"
#include "exerciserecord.hpp"
#include "views/workoutview.hpp"
#include <QHBoxLayout>
#include <QValidator>

ExerciseRecord::ExerciseRecord(int numSets,
                               int minReps,
                               int maxReps,
                               int pauseSeconds,
                               const std::vector<int>& currentReps,
                               Database::WorkoutData::ExerciseEntry* entry,
                               bool isWorkoutFinale,
                               WorkoutView* parent)
    : QWidget(parent),
      numSets_(numSets),
      minReps_(minReps),
      maxReps_(maxReps),
      pauseSeconds_(pauseSeconds),
      currentReps_(currentReps),
      entry_(entry),
      isWorkoutFinale_(isWorkoutFinale),
      parentWorkout_(parent)
{
    if (numSets_ == 0) return;

    parentWorkout_ = parent;

    mainLayout_ = new QVBoxLayout(this);
    mainLayout_->setContentsMargins(8, 8, 8, 8);
    mainLayout_->setSpacing(2);

    // I don't think this is a particularly good test and adjustment, because currentReps has a fixed size;
    // numSets_ could be adjusted downward, but I think it would be better if the constructor fails.
    if (currentReps_.size() < static_cast<size_t>(numSets_))
    {
        currentReps_.resize(numSets_, minReps_);
    }

    createSets();
    createRestTimerAndLabel();
    createWeightWidgets();
}

void ExerciseRecord::createSets()
{
    if (!mainLayout_) return;

    sets_.reserve(numSets_);

    for (int set = 0; set < numSets_; ++set)
    {
        int current = (set < numSets_) ? currentReps_[set] : minReps_;

        auto* setWidget = new ExerciseRecordSet(set, minReps_, maxReps_, current, this);

        setWidget->setEnabled(false);

        mainLayout_->addWidget(setWidget);
        sets_.append(setWidget);
    }

    mainLayout_->addStretch(1);
}

void ExerciseRecord::createRestTimerAndLabel()
{
    if (!mainLayout_) return;

    restTimer_ = new QTimer(this);
    restTimer_->setSingleShot(true);
    connect(restTimer_, &QTimer::timeout, this, &ExerciseRecord::onRestTimerTick);

    restLabel_ = new QLabel(selectRepsText_, this);
    restLabel_->setAlignment(Qt::AlignCenter);

    mainLayout_->addWidget(restLabel_);
}

void ExerciseRecord::createWeightWidgets()
{
    if (!mainLayout_) return;

    auto* weightLayout = new QHBoxLayout();

    QDoubleValidator* validator = new QDoubleValidator(0.0, 1000.0, 2, this);
    validator->setLocale(QLocale::C);

    currentWeightLabel_ = new QLabel("Current weight: ", this);
    currentWeightLabel_->setAlignment(Qt::AlignRight);
    weightLayout->addWidget(currentWeightLabel_);

    currentWeightEdit_ = new QLineEdit(QString::number(entry_->currentWeight, 'f', 2), this);
    currentWeightEdit_->setValidator(validator);
    currentWeightEdit_->setAlignment(Qt::AlignRight);
    currentWeightEdit_->setFixedWidth(110);
    currentWeightEdit_->setEnabled(false);
    weightLayout->addWidget(currentWeightEdit_);

    nextWeightLabel_ = new QLabel("Next weight: ", this);
    nextWeightLabel_->setAlignment(Qt::AlignRight);
    weightLayout->addWidget(nextWeightLabel_);

    nextWeightEdit_ = new QLineEdit(QString::number(entry_->nextWeight, 'f', 2), this);
    nextWeightEdit_->setValidator(validator);
    nextWeightEdit_->setAlignment(Qt::AlignRight);
    nextWeightEdit_->setFixedWidth(110);
    nextWeightEdit_->setEnabled(false);
    weightLayout->addWidget(nextWeightEdit_);

    mainLayout_->addLayout(weightLayout);
}

void ExerciseRecord::enableSet(int setIndex)
{
// I am tempted to set 'currentSetIndex_ = setIndex;' here,
// and to adjust enableRecording() and RestHasEnded() accordingly.
    if (setIndex >= 0 && setIndex < numSets_)
        sets_[setIndex]->setEnabled(true);
}

void ExerciseRecord::disableSet(int setIndex)
{
    if (setIndex >= 0 && setIndex < numSets_)
        sets_[setIndex]->setEnabled(false);
}

void ExerciseRecord::enableRecording()
{
    if (!sets_.isEmpty())
    {
        currentSetIndex_ = 0;
        enableSet(0);
        currentWeightEdit_->setEnabled(true);
    }
}

void ExerciseRecord::updateRepsValue(int setNum, int numReps)
{
    if (setNum != currentSetIndex_) return;  // Replaces: (setNum < 0 || setNum >= numSets_) return;

    currentReps_[setNum] = numReps;

    startRestTimer();
}

void ExerciseRecord::startARestTimerSecond()
{
    restLabel_->setText(QString("%1: %2 seconds").arg(restPrefix_).arg(timerRemainingSeconds_));

    restTimer_->start(1000);
}

void ExerciseRecord::startRestTimer()
{
    if (timerIsRunning_) return;

    timerIsRunning_ = true;

    if (pauseSeconds_ > 0 && restTimer_)
    {
        restPrefix_ = isWorkoutFinale_ ? "Exiting in" : "Rest";
        timerRemainingSeconds_ = pauseSeconds_;
        startARestTimerSecond();
    }

    if (currentSetIndex_ == numSets_ - 1)
    {
        if (entry_)
        {
            nextWeightEdit_->setEnabled(true);
            emit finalRestStarted(entry_);
        }
    }
}

void ExerciseRecord::onRestTimerTick()
{
    --timerRemainingSeconds_;
    if (timerRemainingSeconds_ > 0)
    {
        startARestTimerSecond();
    } else {
        RestHasEnded();
    }
}

void ExerciseRecord::RestHasEnded()
{
    timerIsRunning_ = false;

    disableSet(currentSetIndex_);

    restLabel_->setText(selectRepsText_);

    if (currentSetIndex_ + 1 <= numSets_ - 1)
    {
        currentSetIndex_++;
        enableSet(currentSetIndex_);
    } else {
        // Final rest finished → exercise is fully done
        if (entry_)
        {
            // direct update to model
            // TODO: entry_->warmupWeight = ?;
            entry_->currentWeight = currentWeightEdit_->text().toDouble();
            entry_->setReps = currentReps_;
            entry_->nextWeight = nextWeightEdit_->text().toDouble();
        }

        emit exerciseCompleted(entry_);
    }
}

std::vector<int> ExerciseRecord::currentReps() const
{
    return currentReps_;
}