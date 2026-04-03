#ifndef EXERCISERECORDWARMUP_HPP
#define EXERCISERECORDWARMUP_HPP

#include <QWidget>
#include <QValidator>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>

class ExerciseRecord;

class ExerciseRecordWarmup : public QWidget
{
    Q_OBJECT
public:
    explicit ExerciseRecordWarmup(double weight, QDoubleValidator* validator, ExerciseRecord* parent = nullptr);

    double getWeight() const;

private slots:
    void onButtonClicked();

private:
    void createWidgets(double weight, QDoubleValidator* validator);

    ExerciseRecord* parent_ = nullptr;
    bool hasBeenClicked_ = false;
    QHBoxLayout* layout_ = nullptr;
    QLabel* weightLabel_ = nullptr;
    QLineEdit* weightEdit_ = nullptr;
    QPushButton* button_ = nullptr;
    QLabel* fillerLabel_ = nullptr;
};

#endif