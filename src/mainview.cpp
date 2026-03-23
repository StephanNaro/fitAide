#include "fitAide.hpp"
#include "mainview.hpp"
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

MainView::MainView(Database& db, QWidget* parent)
    : QMainWindow(parent), db_(db), currentExerciseIndex_(0), numSets_(0), pauseSeconds_(0),
      setReps_(), horrible_(0), nameLabel_(nullptr), weightLabel_(nullptr), weightEdit_(nullptr),
      descriptionLabel_(nullptr), imageLabel_(nullptr), setButtons_(), countdownLabel_(nullptr),
      countdownTimer_(nullptr), currentEnabledButtons_(-1), remainingSeconds_(0)
{
    setWindowTitle("fitAide");
    setMinimumWidth(400);

    // Create menubar
    QMenuBar* menuBar = new QMenuBar(this);
    setMenuBar(menuBar);
    QMenu* fileMenu = menuBar->addMenu("File");
    QAction* addExerciseAction = fileMenu->addAction("Add Exercise");
    connect(addExerciseAction, &QAction::triggered, this, &MainView::addExercise);

    QWidget* centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    QVBoxLayout* layout = new QVBoxLayout(centralWidget);

    // Name (centered heading)
    nameLabel_ = new QLabel("No exercise loaded", this);
    nameLabel_->setAlignment(Qt::AlignCenter);
    QFont nameFont = nameLabel_->font();
    nameFont.setBold(true);
    nameLabel_->setFont(nameFont);
    layout->addWidget(nameLabel_);

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
    layout->addLayout(contentLayout);

    // Sets area
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
    layout->addLayout(countdownLayout);
    countdownTimer_ = new QTimer(this);
    connect(countdownTimer_, &QTimer::timeout, this, &MainView::updateCountdown);

    // Initialize
    exerciseIds_ = db_.getExerciseIds();
    if (exerciseIds_.empty()) {
        QMessageBox::critical(this, "Error", "No exercises found");
        close();
        return;
    }
    int minReps, maxReps;
    if (!db_.getSettings(numSets_, minReps, maxReps, pauseSeconds_)) {
        QMessageBox::critical(this, "Error", "Failed to load settings");
        close();
        return;
    }
    setReps_.resize(numSets_, -1);
    setupSetButtons(minReps, maxReps);

    // Load initial exercise and progress
    if (!loadExercise(exerciseIds_[0])) {
        QMessageBox::critical(this, "Error", "Failed to load first exercise");
        close();
        return;
    }
    auto latest = db_.getLatestSessionData(exerciseIds_[0]);
    horrible_ = latest.exerciseId;
    setReps_.resize(numSets_, -1);
    for (int i = 0; i < numSets_ && i < static_cast<int>(latest.setReps.size()); ++i) {
        setReps_[i] = latest.setReps[i];
    }
    weightEdit_->setText(QString::number(latest.currentWeight, 'f', 2));
    // Highlight buttons based on initial progress
    for (int i = 0; i < numSets_; ++i) {
        for (QPushButton* button : setButtons_[i]) {
            int reps = button->text().toInt();
            button->setStyleSheet(""); // Clear previous highlights
            if (i < static_cast<int>(setReps_.size()) && setReps_[i] == reps) {
                button->setStyleSheet("background-color: yellow;");
            }
        }
    }
}

bool MainView::loadExercise(int exerciseId) {
    Database::ExerciseDetails details = db_.getExerciseDetails(exerciseId);
    if (details.name.empty()) {
        return false; // No exercise found
    }

    nameLabel_->setText(QString::fromStdString(details.name));
    if (!details.image.isEmpty()) {
        QImage image;
        if (image.loadFromData(details.image)) {
            imageLabel_->setPixmap(QPixmap::fromImage(image.scaled(200, 200, Qt::KeepAspectRatio, Qt::SmoothTransformation)));
        } else {
            imageLabel_->setText("Invalid image");
        }
    } else {
        imageLabel_->setText("No image");
    }
    descriptionLabel_->setText(details.description.empty() ? "No description" : QString::fromStdString(details.description));
    weightLabel_->setText(QString("Weight: %1 kg").arg(QString::number(details.currentWeight, 'f', 2)));

    return true;
}

void MainView::setupSetButtons(int minReps, int maxReps) {
    QVBoxLayout* layout = qobject_cast<QVBoxLayout*>(centralWidget()->layout());
    setButtons_.resize(numSets_);
    for (int i = 0; i < numSets_; ++i) {
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
    currentEnabledButtons_ = 0;
}

void MainView::onRepsButtonClicked(int setIndex, int reps) {
    setReps_[setIndex] = reps;
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
        countdownLabel_->setText(QString("Rest: %1 seconds").arg(pauseSeconds_));
        remainingSeconds_ = pauseSeconds_;
        countdownTimer_->start(1000);

        if (setIndex == numSets_ - 1) {
            weightEdit_->setVisible(true); // Show weightEdit_ for last set
            // Load next exercise for UI refresh
            currentExerciseIndex_++;
            if (currentExerciseIndex_ < exerciseIds_.size()) {
                if (!loadExercise(exerciseIds_[currentExerciseIndex_])) {
                    QMessageBox::critical(this, "Error", "Failed to load next exercise");
                    close();
                    return;
                }
            }
        }
    }
}

void MainView::updateCountdown() {
    remainingSeconds_--;
    countdownLabel_->setText(QString("Rest: %1 seconds").arg(remainingSeconds_));
    if (remainingSeconds_ <= 0) {
        countdownTimer_->stop();
        countdownLabel_->setText("");
        weightEdit_->setVisible(false); // Hide weightEdit_ when countdown ends

        // Disable current set
        if (currentEnabledButtons_ >= 0 && currentEnabledButtons_ < numSets_) {
            for (QPushButton* button : setButtons_[currentEnabledButtons_]) {
                button->setEnabled(false);
            }
        }

        // Enable next set
        currentEnabledButtons_++;
        if (currentEnabledButtons_ >= numSets_) {
            // Save progress only for the last set
            saveProgress();
            // Update progress for next exercise
            if (currentExerciseIndex_ < exerciseIds_.size()) {
                Database::LatestSessionData latest = db_.getLatestSessionData(exerciseIds_[currentExerciseIndex_]);
                horrible_ = latest.exerciseId;
                setReps_.resize(numSets_, -1);
                for (int i = 0; i < numSets_ && i < static_cast<int>(latest.setReps.size()); ++i) {
                    setReps_[i] = latest.setReps[i];
                }
                // Highlight buttons for next exercise
                for (int i = 0; i < numSets_; ++i) {
                    for (QPushButton* button : setButtons_[i]) {
                        int reps = button->text().toInt();
                        button->setStyleSheet(""); // Clear previous highlights
                        if (i < static_cast<int>(setReps_.size()) && setReps_[i] == reps) {
                            button->setStyleSheet("background-color: yellow;");
                        }
                    }
                }
                // Set weightEdit_ for next exercise
                weightEdit_->setText(QString::number(latest.currentWeight, 'f', 2));
            }
            // Exit after last set of last exercise
            if (currentExerciseIndex_ >= exerciseIds_.size()) {
                close();
            }
            currentEnabledButtons_ = 0;
        }
        for (QPushButton* button : setButtons_[currentEnabledButtons_]) {
            button->setEnabled(true);
        }
    }
}

void MainView::saveProgress() {
    double weight = weightEdit_->text().toDouble();
    int reps[5] = {-1, -1, -1, -1, -1};
    for (size_t i = 0; i < setReps_.size() && i < 5; ++i) {
        reps[static_cast<int>(i)] = setReps_[i];
    }
    QString sessionTime = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
    if (!db_.insertSessionEntry(
            horrible_,  // This might be correct later: exerciseIds_[currentExerciseIndex_],
            weight,
            reps[0], reps[1], reps[2], reps[3], reps[4],
            sessionTime.toStdString())) {
        QMessageBox::critical(this, "Error", "Failed to save session entry");
    }
    setReps_.assign(numSets_, -1);   // reset
}

void MainView::nextExercise() {
    // No UI refresh here; handled in onRepsButtonClicked
    if (currentExerciseIndex_ >= exerciseIds_.size()) {
        close();
    }
    weightEdit_->setText("0.0");
}

void MainView::addExercise() {
    ExerciseDialog dialog(db_, this);
    if (dialog.exec() == QDialog::Accepted) {
        // Reload exercise IDs to include new exercises
        exerciseIds_ = db_.getExerciseIds();
        if (exerciseIds_.empty()) {
            QMessageBox::critical(this, "Error", "No exercises found after adding");
            close();
        }
    }
}