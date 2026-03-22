#include "fitAide.hpp"
#include "routinedialog.hpp"
#include <QtWidgets>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>

RoutineDialog::RoutineDialog(Database& db, QWidget* parent)
    : QDialog(parent), db_(db) {
    setWindowTitle("Enter Routine");
    setMinimumWidth(400);
    auto* layout = new QVBoxLayout(this);

    // NumSets
    numSetsSpin_ = new QSpinBox(this);
    numSetsSpin_->setRange(2, 5);
    numSetsSpin_->setValue(3);
    numSetsSlider_ = new QSlider(Qt::Horizontal, this);
    numSetsSlider_->setRange(2, 5);
    numSetsSlider_->setValue(3);
    connect(numSetsSlider_, &QSlider::valueChanged, numSetsSpin_, &QSpinBox::setValue);
    connect(numSetsSpin_, &QSpinBox::valueChanged, numSetsSlider_, &QSlider::setValue);
    layout->addWidget(new QLabel("Number of Sets (2-5):", this));
    layout->addWidget(numSetsSpin_);
    layout->addWidget(numSetsSlider_);

    // MinReps
    minRepsSpin_ = new QSpinBox(this);
    minRepsSpin_->setRange(1, 20);
    minRepsSpin_->setValue(8);
    minRepsSlider_ = new QSlider(Qt::Horizontal, this);
    minRepsSlider_->setRange(1, 20);
    minRepsSlider_->setValue(8);
    connect(minRepsSlider_, &QSlider::valueChanged, minRepsSpin_, &QSpinBox::setValue);
    connect(minRepsSpin_, &QSpinBox::valueChanged, minRepsSlider_, &QSlider::setValue);
    layout->addWidget(new QLabel("Minimum Reps (1-20):", this));
    layout->addWidget(minRepsSpin_);
    layout->addWidget(minRepsSlider_);

    // MaxReps
    maxRepsSpin_ = new QSpinBox(this);
    maxRepsSpin_->setRange(1, 20);
    maxRepsSpin_->setValue(12);
    maxRepsSlider_ = new QSlider(Qt::Horizontal, this);
    maxRepsSlider_->setRange(1, 20);
    maxRepsSlider_->setValue(12);
    connect(maxRepsSlider_, &QSlider::valueChanged, maxRepsSpin_, &QSpinBox::setValue);
    connect(maxRepsSpin_, &QSpinBox::valueChanged, maxRepsSlider_, &QSlider::setValue);
    layout->addWidget(new QLabel("Maximum Reps (1-20):", this));
    layout->addWidget(maxRepsSpin_);
    layout->addWidget(maxRepsSlider_);

    // Pause
    pauseSpin_ = new QSpinBox(this);
    pauseSpin_->setRange(120, 300);
    pauseSpin_->setValue(120);
    pauseSpin_->setSuffix(" seconds");
    pauseSlider_ = new QSlider(Qt::Horizontal, this);
    pauseSlider_->setRange(120, 300);
    pauseSlider_->setValue(120);
    connect(pauseSlider_, &QSlider::valueChanged, pauseSpin_, &QSpinBox::setValue);
    connect(pauseSpin_, &QSpinBox::valueChanged, pauseSlider_, &QSlider::setValue);
    layout->addWidget(new QLabel("Pause Between Sets (120-300):", this));
    layout->addWidget(pauseSpin_);
    layout->addWidget(pauseSlider_);

    // Done Button
    auto* buttonLayout = new QHBoxLayout();
    doneButton_ = new QPushButton("Done", this);
    connect(doneButton_, &QPushButton::clicked, this, &RoutineDialog::onDoneClicked);
    buttonLayout->addStretch();
    buttonLayout->addWidget(doneButton_);
    layout->addLayout(buttonLayout);
}

bool RoutineDialog::saveRoutine() {
    int numSets = numSetsSpin_->value();
    int minReps = minRepsSpin_->value();
    int maxReps = maxRepsSpin_->value();
    int pause = pauseSpin_->value();

    if (minReps > maxReps) {
        QMessageBox::warning(this, "Error", "Minimum reps cannot exceed maximum reps");
        return false;
    }

    sqlite3_stmt* stmt;
    const char* query = "INSERT INTO Routine (NumSets, MinReps, MaxReps, Pause) VALUES (?, ?, ?, ?);";
    if (sqlite3_prepare_v2(db_.getDb(), query, -1, &stmt, nullptr) != SQLITE_OK) {
        QMessageBox::critical(this, "Error", QString("Failed to prepare statement: %1").arg(sqlite3_errmsg(db_.getDb())));
        return false;
    }

    sqlite3_bind_int(stmt, 1, numSets);
    sqlite3_bind_int(stmt, 2, minReps);
    sqlite3_bind_int(stmt, 3, maxReps);
    sqlite3_bind_int(stmt, 4, pause);

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        QMessageBox::critical(this, "Error", QString("Failed to save routine: %1").arg(sqlite3_errmsg(db_.getDb())));
        sqlite3_finalize(stmt);
        return false;
    }
    sqlite3_finalize(stmt);
    return true;
}

void RoutineDialog::onDoneClicked() {
    if (saveRoutine()) {
        accept();
    }
}