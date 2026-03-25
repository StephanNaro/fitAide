#include "fitAide.hpp"
#include "workoutview.hpp"
#include "exercisedialog.hpp"
#include <QApplication>
#include <QtWidgets>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPixmap>
#include <QMessageBox>
#include <QDateTime>
#include <QLineEdit>
#include <QLocale>
#include <QMenuBar>
#include <iostream>

WorkoutView::WorkoutView(Database& db, QWidget* parent)
    : QMainWindow(parent), db_(db), uiExerciseIndex_(0), saveIndex_(0),
      nameLabel_(nullptr), weightLabel_(nullptr), weightEdit_(nullptr),
      descriptionLabel_(nullptr), imageLabel_(nullptr),
      setButtons_(), currentEnabledButtons_(-1),
      countdownLabel_(nullptr), countdownTimer_(nullptr), remainingSeconds_(0),
      workoutTime_("")
{
    setWindowTitle("fitAide");
    setMinimumWidth(400);

    // Initialize
    workoutData_ = db_.loadFullWorkoutData();
    if (workoutData_.exercises.empty()) {
        QMessageBox::critical(this, "Error", "No active exercises found");
        close();
        return;
    }
    workoutTime_ = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
    createMenuAndLayout();
    createSetButtons(workoutData_.minReps, workoutData_.maxReps);

    populateNameDescriptionImageWeight(uiExerciseIndex_);
    weightEdit_->setText(QString::number(workoutData_.exercises[uiExerciseIndex_].currentWeight, 'f', 2));

    // Highlight buttons based on initial progress
    for (int i = 0; i < workoutData_.numSets; ++i) {
        for (QPushButton* btn : setButtons_[i]) {
            int reps = btn->text().toInt();
            if (   i < static_cast<int>(workoutData_.exercises[uiExerciseIndex_].setReps.size())
                && workoutData_.exercises[uiExerciseIndex_].setReps[i] == reps) {
                btn->setStyleSheet("background-color: yellow;");
            }
        }
    }
}

void WorkoutView::createMenuAndLayout() {
    QMenuBar* menuBar = new QMenuBar(this);
    setMenuBar(menuBar);
    QMenu* fileMenu = menuBar->addMenu("File");
    QAction* addExerciseAction = fileMenu->addAction("Add Exercise");
    connect(addExerciseAction, &QAction::triggered, this, &WorkoutView::addExercise);

    QWidget* central = new QWidget(this);
    setCentralWidget(central);
    QVBoxLayout* mainLayout = new QVBoxLayout(central);

    nameLabel_ = new QLabel("No exercise loaded", this);
    nameLabel_->setAlignment(Qt::AlignCenter);
    QFont f = nameLabel_->font();
    f.setBold(true);
    nameLabel_->setFont(f);
    mainLayout->addWidget(nameLabel_);

    // Weight and Description (left), Image (right)
    QHBoxLayout* contentLayout = new QHBoxLayout();
    QVBoxLayout* leftLayout = new QVBoxLayout();
    weightLabel_ = new QLabel("Weight: 0.0 kg", this);
    weightLabel_->setAlignment(Qt::AlignCenter);
    leftLayout->addWidget(weightLabel_);
    descriptionLabel_ = new QLabel("No description", this);
    descriptionLabel_->setWordWrap(true);
    leftLayout->addWidget(descriptionLabel_);
    leftLayout->addStretch();
    contentLayout->addLayout(leftLayout);
    imageLabel_ = new QLabel("No image", this);
    imageLabel_->setFixedSize(200, 200);
    imageLabel_->setAlignment(Qt::AlignCenter);
    imageLabel_->setFrameStyle(QFrame::Box | QFrame::Sunken);
    contentLayout->addWidget(imageLabel_);
    mainLayout->addLayout(contentLayout);

    // Sets area: countdown + weightEdit
    QHBoxLayout* countdownLayout = new QHBoxLayout();
    countdownLabel_ = new QLabel("", this);
    countdownLabel_->setAlignment(Qt::AlignCenter);
    countdownLayout->addWidget(countdownLabel_);
    weightEdit_ = new QLineEdit("0.0", this);
    QDoubleValidator* validator = new QDoubleValidator(0.0, 1000.0, 2, this);
    validator->setLocale(QLocale::C); // Use dot (.) as decimal separator
    weightEdit_->setValidator(validator);
    weightEdit_->setAlignment(Qt::AlignRight);
    weightEdit_->setVisible(false); // Initially hidden
    countdownLayout->addWidget(weightEdit_);
    mainLayout->addLayout(countdownLayout);

    countdownTimer_ = new QTimer(this);
    connect(countdownTimer_, &QTimer::timeout, this, &WorkoutView::updateCountdown);
}

void WorkoutView::createSetButtons(int minReps, int maxReps) {
    QVBoxLayout* layout = qobject_cast<QVBoxLayout*>(centralWidget()->layout());
    setButtons_.resize(workoutData_.numSets);
    for (int i = 0; i < workoutData_.numSets; ++i) {
        QHBoxLayout* setLayout = new QHBoxLayout();
        setButtons_[i].clear();
        for (int reps = minReps; reps <= maxReps; ++reps) {
            QPushButton* button = new QPushButton(QString::number(reps), this);
            button->setEnabled(i == 0); // Enable only first set
            connect(button, &QPushButton::clicked, this, [this, i, reps]() { onRepsButtonClicked(i, reps); });
            setButtons_[i].append(button);
            setLayout->addWidget(button);
        }
        layout->insertLayout(layout->count() - 1, setLayout); // Before countdown
    }
    saveIndex_ = uiExerciseIndex_;
    currentEnabledButtons_ = 0;
}

void WorkoutView::populateNameDescriptionImageWeight(size_t index) {
    if (index >= workoutData_.exercises.size()) return;

    const auto& ex = workoutData_.exercises[index];
    nameLabel_->setText(QString::fromStdString(ex.name));
    descriptionLabel_->setText(ex.description.empty() ? "No description" : QString::fromStdString(ex.description));

    if (!ex.image.isEmpty()) {
        QImage img;
        if (img.loadFromData(ex.image)) {
            imageLabel_->setPixmap(QPixmap::fromImage(img.scaled(200, 200, Qt::KeepAspectRatio, Qt::SmoothTransformation)));
        } else {
            imageLabel_->setText("Invalid image");
        }
    } else {
        imageLabel_->setText("No image");
    }

    weightLabel_->setText(QString("Weight: %1 kg").arg(ex.currentWeight, 0, 'f', 2));
}

void WorkoutView::onRepsButtonClicked(int setIndex, int reps) {
    workoutData_.exercises[saveIndex_].setReps[setIndex] = reps;
    // Reset stylesheet for all buttons in this set
    for (QPushButton* button : setButtons_[setIndex]) {
        button->setStyleSheet("");
    }
    // Highlight clicked button green
    for (QPushButton* button : setButtons_[setIndex]) {
        if (button->text().toInt() == reps) {
            button->setStyleSheet("background-color: green;");
            break;
        }
    }
    if (!countdownTimer_->isActive()) {
        countdownLabel_->setText(QString("Rest: %1 seconds").arg(workoutData_.pauseSeconds));
        remainingSeconds_ = workoutData_.pauseSeconds;
        countdownTimer_->start(1000);

        if (setIndex == workoutData_.numSets - 1) {
            weightEdit_->setVisible(true); // Show weightEdit_ for final set
            // Populate UI for next exercise
            uiExerciseIndex_++;
            if (uiExerciseIndex_ < workoutData_.exercises.size()) {
                populateNameDescriptionImageWeight(uiExerciseIndex_);
            }
        }
    }
}

void WorkoutView::updateCountdown() {
    remainingSeconds_--;
    countdownLabel_->setText(QString("Rest: %1 seconds").arg(remainingSeconds_));
    if (remainingSeconds_ <= 0) {
        countdownTimer_->stop();
        countdownLabel_->setText("");
        weightEdit_->setVisible(false); // Hide weightEdit_ when countdown ends

        // Disable current set
        if (currentEnabledButtons_ >= 0 && currentEnabledButtons_ < workoutData_.numSets) {
            for (QPushButton* button : setButtons_[currentEnabledButtons_]) {
                button->setEnabled(false);
            }
        }

        // Enable next set's buttons
        currentEnabledButtons_++;
        if (currentEnabledButtons_ >= workoutData_.numSets) {
            // Save progress only after the final set
            saveProgress();
            if (saveIndex_ < workoutData_.exercises.size()) {
                // Highlight buttons for next exercise
                for (int i = 0; i < workoutData_.numSets; ++i) {
                    for (QPushButton* button : setButtons_[i]) {
                        int reps = button->text().toInt();
                        button->setStyleSheet(""); // Clear previous highlights
                        if (   i < static_cast<int>(workoutData_.exercises[saveIndex_].setReps.size())
                            && workoutData_.exercises[saveIndex_].setReps[i] == reps) {
                            button->setStyleSheet("background-color: yellow;");
                        }
                    }
                }
                weightEdit_->setText(QString::number(workoutData_.exercises[saveIndex_].currentWeight, 'f', 2));
            }
            // Exit program after final set of final exercise
            if (uiExerciseIndex_ >= workoutData_.exercises.size()) {
                close();
            }
            currentEnabledButtons_ = 0;
        }
        for (QPushButton* button : setButtons_[currentEnabledButtons_]) {
            button->setEnabled(true);
        }
    }
}

void WorkoutView::saveProgress() {
    double weight = weightEdit_->text().toDouble();
    int reps[5] = {-1, -1, -1, -1, -1};
    for (size_t i = 0; i < workoutData_.exercises[saveIndex_].setReps.size() && i < 5; ++i) {
        reps[static_cast<int>(i)] = workoutData_.exercises[saveIndex_].setReps[i];
    }
    //QString workoutTime = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
    if (!db_.insertWorkoutEntry(
            workoutData_.exercises[saveIndex_].exerciseId,
            weight,
            reps[0], reps[1], reps[2], reps[3], reps[4],
            //workoutTime.toStdString())) {  // Temporary
            workoutTime_.toStdString())) {   // Temporary
        QMessageBox::critical(this, "Error", "Failed to save workout entry");
    }
    saveIndex_ = uiExerciseIndex_;
}

void WorkoutView::addExercise() {
    ExerciseDialog dialog(db_, this);
}