#include "exercisehead.hpp"
#include <QVBoxLayout>

ExerciseHead::ExerciseHead(const QString& heading, QWidget* parent)
    : QWidget(parent)
{
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    headingLabel_ = new QLabel(heading, this);
    headingLabel_->setAlignment(Qt::AlignCenter);

    QFont font = headingLabel_->font();
    font.setBold(true);
    font.setPointSize(font.pointSize() + 8);
    headingLabel_->setFont(font);

    layout->addWidget(headingLabel_);
}

void ExerciseHead::setHeading(const QString& heading) {
    headingLabel_->setText(heading);
}