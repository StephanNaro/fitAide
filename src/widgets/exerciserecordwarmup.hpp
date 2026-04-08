#ifndef EXERCISERECORDWARMUP_HPP
#define EXERCISERECORDWARMUP_HPP

#include <QWidget>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>

class ExerciseRecord;

class ExerciseRecordWarmup : public QWidget
{
    Q_OBJECT
public:
    explicit ExerciseRecordWarmup(double weight, ExerciseRecord* parent = nullptr);

private slots:
    void onButtonClicked();

private:
    void createWidgets(double weight);

    ExerciseRecord* parent_ = nullptr;
    QHBoxLayout* layout_ = nullptr;
    QLabel* weightLabel_ = nullptr;
    QPushButton* button_ = nullptr;
    QLabel* fillerLabel_ = nullptr;
};

#endif