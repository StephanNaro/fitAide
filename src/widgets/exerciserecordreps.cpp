#include "exerciserecordreps.hpp"
#include "exerciserecordset.hpp"

ExerciseRecordReps::ExerciseRecordReps(int numReps, bool isHighlighted, ExerciseRecordSet* parent)
    : QPushButton(QString::number(numReps), parent),
    parent_(parent),
    numReps_(numReps)
{
    setStyleSheet(isHighlighted ? "background-color: yellow;" : "");
    connect(this, &QPushButton::clicked, this, &ExerciseRecordReps::onButtonClicked);
}

void ExerciseRecordReps::onButtonClicked()
{
    parent_->setReps(numReps_, this);
    setStyleSheet("background-color: green;");
}

void ExerciseRecordReps::Unselect()
{
    setStyleSheet("");
}