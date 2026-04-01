#ifndef EXERCISEDIALOG_HPP
#define EXERCISEDIALOG_HPP

#include "database/database.hpp"
#include <QDialog>
#include <QLineEdit>
#include <QLabel>
#include <QTextEdit>
#include <QPushButton>
#include <QByteArray>

class ExerciseDialog : public QDialog
{
    Q_OBJECT
public:
    explicit ExerciseDialog(Database& db, QWidget* parent = nullptr);
    ~ExerciseDialog() = default;

private slots:
    void onNextClicked();
    void onDoneClicked();
    void onPasteImage();

private:
    Database& db_;
    QLineEdit* nameEdit_;
    QLabel* imageLabel_;
    QTextEdit* descriptionEdit_;
    QPushButton* nextButton_;
    QPushButton* doneButton_;
    QByteArray imageData_;

    bool saveExercise();
};

#endif