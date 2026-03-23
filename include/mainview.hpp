#ifndef MAINVIEW_HPP
#define MAINVIEW_HPP

#include "database.hpp"
#include <QMainWindow>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QTimer>
#include <vector>

class MainView : public QMainWindow {
    Q_OBJECT
public:
    explicit MainView(Database& db, QWidget* parent = nullptr);
    ~MainView() = default;

private slots:
    void onRepsButtonClicked(int setIndex, int reps);
    void updateCountdown();
    void addExercise();

private:
    Database& db_;
    std::vector<int> exerciseIds_;
    size_t currentExerciseIndex_;
    int numSets_;
    int pauseSeconds_;
    std::vector<int> setReps_;
    int horrible_;  // Doesn't work without this. Should be obviated once all data is loaded, and saved, in one go.
    QLabel* nameLabel_;
    QLabel* weightLabel_;
    QLineEdit* weightEdit_;
    QLabel* descriptionLabel_;
    QLabel* imageLabel_;
    std::vector<QList<QPushButton*>> setButtons_;
    QLabel* countdownLabel_;
    QTimer* countdownTimer_;
    int currentEnabledButtons_;
    int remainingSeconds_;

    bool loadExercise(int exerciseId);
    void setupSetButtons(int minReps, int maxReps);
    void saveProgress();
    void nextExercise();
};

#endif // MAINVIEW_HPP