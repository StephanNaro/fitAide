#ifndef ROUTINEDIALOG_HPP
#define ROUTINEDIALOG_HPP

#include "database.hpp"
#include <QDialog>
#include <QSlider>
#include <QSpinBox>
#include <QPushButton>

class RoutineDialog : public QDialog {
    Q_OBJECT
public:
    explicit RoutineDialog(Database& db, QWidget* parent = nullptr);
    ~RoutineDialog() = default;

private slots:
    void onDoneClicked();

private:
    Database& db_;
    QSpinBox* numSetsSpin_;
    QSlider* numSetsSlider_;
    QSpinBox* minRepsSpin_;
    QSlider* minRepsSlider_;
    QSpinBox* maxRepsSpin_;
    QSlider* maxRepsSlider_;
    QSpinBox* pauseSpin_;
    QSlider* pauseSlider_;
    QPushButton* doneButton_;

    bool saveRoutine();
};

#endif // ROUTINEDIALOG_HPP