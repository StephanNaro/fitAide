#ifndef EXERCISESTATUS_HPP
#define EXERCISESTATUS_HPP

#include <QWidget>
#include <QString>
#include <QLabel>

class ExerciseStatus : public QWidget
{
    Q_OBJECT
public:
    explicit ExerciseStatus(const QString& status = "Upcoming...", QWidget* parent = nullptr);
    void setStatus(const QString& status);

private:
    QLabel* statusLabel_;
};

#endif