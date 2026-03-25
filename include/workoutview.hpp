#ifndef WORKOUTVIEW_HPP
#define WORKOUTVIEW_HPP

#include "database.hpp"
#include <QMainWindow>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QTimer>
#include <vector>

class WorkoutView : public QMainWindow {
    Q_OBJECT
public:
    explicit WorkoutView(Database& db, QWidget* parent = nullptr);
    ~WorkoutView() = default;

private slots:
    void onRepsButtonClicked(int setIndex, int reps);
    void updateCountdown();
    void addExercise();

private:
    Database& db_;
    Database::WorkoutData workoutData_;
    size_t uiExerciseIndex_ = 0;
    size_t saveIndex_ = 0;

    QLabel* nameLabel_;
    QLabel* weightLabel_;
    QLineEdit* weightEdit_;
    QLabel* descriptionLabel_;
    QLabel* imageLabel_;

    std::vector<QList<QPushButton*>> setButtons_;
    int currentEnabledButtons_;

    QLabel* countdownLabel_;
    QTimer* countdownTimer_;
    int remainingSeconds_;

    bool workoutInProgress_ = false;

    void createMenuAndLayout();
    void createSetButtons(int minReps, int maxReps);
    void populateNameDescriptionImageWeight(size_t index);
    void highlightButtons(size_t index);
};

#endif // WORKOUTVIEW_HPP