#include "exerciserecordwarmup.hpp"
#include "exerciserecord.hpp"

#include <format>
#include <iostream>

ExerciseRecordWarmup::ExerciseRecordWarmup(double weight, ExerciseRecord* parent)
    : QWidget(parent),
      parent_(parent)
{
    layout_ = new QHBoxLayout(this);
    layout_->setContentsMargins(4, 4, 4, 4);
    layout_->setSpacing(6);
    createWidgets(weight);
    this->setEnabled(false);
}

void ExerciseRecordWarmup::createWidgets(double weight)
{
    if (!layout_) return;

    double ten = weight * 0.10;
    double twenty = weight * 0.20;

    fillerLabel_ = new QLabel("", this);

    weightLabel_ = new QLabel(QString::fromStdString(std::format("Remove <b>{:.2f}</b> <small>- {:.2f}</small> kg for warmup of", ten, twenty)), this);
    weightLabel_->setTextFormat(Qt::RichText);
    weightLabel_->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    button_ = new QPushButton("3-5 Reps", this);
    weightLabel_->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    connect(button_, &QPushButton::clicked, this, &ExerciseRecordWarmup::onButtonClicked);

    layout_->addWidget(fillerLabel_);
    layout_->addWidget(weightLabel_);
    layout_->addWidget(button_);
}

void ExerciseRecordWarmup::onButtonClicked()
{
    if (!parent_) return;
    button_->setEnabled(false);
    parent_->startWarmupRest();
}