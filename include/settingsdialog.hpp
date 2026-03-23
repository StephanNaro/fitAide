#ifndef SETTINGSDIALOG_HPP
#define SETTINGSDIALOG_HPP

#include "database.hpp"
#include <QDialog>
#include <QSlider>
#include <QSpinBox>
#include <QPushButton>

class SettingsDialog : public QDialog {
    Q_OBJECT
public:
    explicit SettingsDialog(Database& db, QWidget* parent = nullptr);
    ~SettingsDialog() = default;
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
    bool saveSettings();
};

#endif // SETTINGSDIALOG_HPP