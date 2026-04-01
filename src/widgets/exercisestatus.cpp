#include "exercisestatus.hpp"
#include <QVBoxLayout>

ExerciseStatus::ExerciseStatus(const QString& status, QWidget* parent)
    : QWidget(parent)
{
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    statusLabel_ = new QLabel(status, this);
    statusLabel_->setAlignment(Qt::AlignCenter);

    QFont font = statusLabel_->font();
    font.setBold(true);
    font.setPointSize(font.pointSize() + 4);
    statusLabel_->setFont(font);
    setStyleSheet("background-color: skyblue;");

    layout->addWidget(statusLabel_);
}

void ExerciseStatus::setStatus(const QString& status) {
    statusLabel_->setText(status);
}