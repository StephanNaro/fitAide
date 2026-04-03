#include "exerciserecordwarmup.hpp"
#include "exerciserecord.hpp"

ExerciseRecordWarmup::ExerciseRecordWarmup(double weight, QDoubleValidator* validator, ExerciseRecord* parent)
    : QWidget(parent),
      parent_(parent),
      hasBeenClicked_(false)
{
    if (!validator) return;

    layout_ = new QHBoxLayout(this);
    layout_->setContentsMargins(4, 4, 4, 4);
    layout_->setSpacing(6);
    createWidgets(weight, validator);
    this->setEnabled(false);
}

void ExerciseRecordWarmup::createWidgets(double weight, QDoubleValidator* validator)
{
    if (!validator || !layout_) return;

    weightLabel_ = new QLabel("Warmup weight: ", this);
    weightLabel_->setAlignment(Qt::AlignRight);

    weightEdit_ = new QLineEdit(QString::number(weight, 'f', 2), this);
    weightEdit_->setValidator(validator);
    weightEdit_->setAlignment(Qt::AlignRight);
    weightEdit_->setFixedWidth(110);

    button_ = new QPushButton("Warmed Up", this);
    connect(button_, &QPushButton::clicked, this, &ExerciseRecordWarmup::onButtonClicked);

    fillerLabel_ = new QLabel("", this);

    layout_->addWidget(weightLabel_);
    layout_->addWidget(weightEdit_);
    layout_->addWidget(button_);
    layout_->addWidget(fillerLabel_);
}

double ExerciseRecordWarmup::getWeight() const
{
    return hasBeenClicked_ ? weightEdit_->text().toDouble() : 0.0;
}

void ExerciseRecordWarmup::onButtonClicked()
{
    if (!parent_) return;
    button_->setEnabled(false);
    hasBeenClicked_ = true;
    parent_->startWarmupRest();
}