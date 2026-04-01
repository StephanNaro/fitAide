#ifndef EXERCISEHEAD_HPP
#define EXERCISEHEAD_HPP

#include <QWidget>
#include <QString>
#include <QLabel>

class ExerciseHead : public QWidget
{
    Q_OBJECT
public:
    explicit ExerciseHead(const QString& heading, QWidget* parent = nullptr);
    void setHeading(const QString& heading);

private:
    QLabel* headingLabel_;
};

#endif