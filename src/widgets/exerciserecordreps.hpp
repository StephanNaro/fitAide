#ifndef EXERCISERECORDREPS_HPP
#define EXERCISERECORDREPS_HPP

#include <QPushButton>

class ExerciseRecordSet;

class ExerciseRecordReps : public QPushButton
{
    Q_OBJECT
public:
    explicit ExerciseRecordReps(int numReps, bool isHighlighted, ExerciseRecordSet* parent = nullptr);
    void Unselect();

private slots:
    void onButtonClicked();

private:
    ExerciseRecordSet* parent_ = nullptr;
    int numReps_ = 0;
};

#endif