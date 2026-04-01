#ifndef EXERCISERECORDSET_HPP
#define EXERCISERECORDSET_HPP

#include <QWidget>
#include <QVector>

class ExerciseRecordReps;
class ExerciseRecord;

class ExerciseRecordSet : public QWidget
{
    Q_OBJECT
public:
    explicit ExerciseRecordSet(int setNum,
                               int minReps,
                               int maxReps,
                               int currentReps,
                               ExerciseRecord* parent = nullptr);

    void setReps(int numReps, ExerciseRecordReps* button);

    int setNumber() const { return setNum_; }
    int currentReps() const { return currentReps_; }

signals:
    void repsChanged(int setNum, int numReps);

private:
    void createRepButtons();

    int setNum_;
    int minReps_;
    int maxReps_;
    int currentReps_;

    ExerciseRecordReps* currentButton_ = nullptr;
    QVector<ExerciseRecordReps*> repButtons_;   // For safety & future use

    ExerciseRecord* parentExercise_ = nullptr;
};

#endif