#include "settingsdialog.hpp"
#include <QVBoxLayout>
#include <QLabel>
#include <QMessageBox>

SettingsDialog::SettingsDialog(Database& db, QWidget* parent)
    : QDialog(parent), db_(db)
{
    setWindowTitle("Workout Settings");
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

    // Rest
    restSpin_ = new QSpinBox(this);
    restSpin_->setRange(120, 300);
    restSpin_->setValue(120);
    restSpin_->setSuffix(" seconds");
    restSlider_ = new QSlider(Qt::Horizontal, this);
    restSlider_->setRange(120, 300);
    restSlider_->setValue(120);
    connect(restSlider_, &QSlider::valueChanged, restSpin_, &QSpinBox::setValue);
    connect(restSpin_, &QSpinBox::valueChanged, restSlider_, &QSlider::setValue);
    layout->addWidget(new QLabel("Rest Between Sets (120-300):", this));
    layout->addWidget(restSpin_);
    layout->addWidget(restSlider_);

    // Done Button
    auto* buttonLayout = new QHBoxLayout();
    doneButton_ = new QPushButton("Save", this);
    connect(doneButton_, &QPushButton::clicked, this, &SettingsDialog::onDoneClicked);
    buttonLayout->addStretch();
    buttonLayout->addWidget(doneButton_);
    layout->addLayout(buttonLayout);
}

bool SettingsDialog::saveSettings()
{
    int numSets = numSetsSpin_->value();
    int minReps = minRepsSpin_->value();
    int maxReps = maxRepsSpin_->value();
    int restSeconds = restSpin_->value();
    if (minReps > maxReps)
    {
        QMessageBox::warning(this, "Error", "Minimum reps cannot exceed maximum reps");
        return false;
    }
    return db_.insertSettings(numSets, minReps, maxReps, restSeconds);
}

void SettingsDialog::onDoneClicked()
{
    if (saveSettings())
    {
        accept();
    }
}