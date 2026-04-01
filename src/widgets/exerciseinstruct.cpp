#include "exerciseinstruct.hpp"
#include <QHBoxLayout>
#include <QVBoxLayout>

ExerciseInstruct::ExerciseInstruct(const QString& description, const QByteArray& imageData, QWidget* parent)
    : QWidget(parent)
{
    auto* mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    // Left side: Description
    auto* leftLayout = new QVBoxLayout();

    descriptionLabel_ = new QLabel(description.isEmpty() ? "No description" : description, this);
    descriptionLabel_->setWordWrap(true);

    leftLayout->addWidget(descriptionLabel_);
    leftLayout->addStretch();

    // Right side: Image
    imageLabel_ = new QLabel("No image", this);
    imageLabel_->setFixedSize(200, 200);
    imageLabel_->setAlignment(Qt::AlignCenter);
    imageLabel_->setFrameStyle(QFrame::Box | QFrame::Sunken);
    setImage(imageData);

    mainLayout->addLayout(leftLayout);
    mainLayout->addWidget(imageLabel_);
}

void ExerciseInstruct::setDescription(const QString& description) {
    descriptionLabel_->setText(description.isEmpty() ? "No description" : description);
}

void ExerciseInstruct::setImage(const QByteArray& imageData) {
    if (!imageData.isEmpty()) {
        QImage img;
        if (img.loadFromData(imageData)) {
            imageLabel_->setPixmap(QPixmap::fromImage(
                img.scaled(200, 200, Qt::KeepAspectRatio, Qt::SmoothTransformation)));
            return;
        }
    }
    imageLabel_->setText("No image");
}