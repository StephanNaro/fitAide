#include "fitAide.hpp"
#include "exercisedialog.hpp"
#include <QtWidgets>
#include <QLineEdit>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QClipboard>
#include <QImage>
#include <QPixmap>
#include <QBuffer>
#include <QMessageBox>
#include <QFontMetrics>

ExerciseDialog::ExerciseDialog(Database& db, QWidget* parent)
    : QDialog(parent), db_(db), imageData_() {
    setWindowTitle("Enter Exercise");
    setMinimumWidth(400); // Improved layout
    auto* layout = new QVBoxLayout(this);

    // Name
    nameEdit_ = new QLineEdit(this);
    nameEdit_->setPlaceholderText("Exercise Name (required)");
    layout->addWidget(new QLabel("Name:", this));
    layout->addWidget(nameEdit_);

    // Image
    imageLabel_ = new QLabel("No image", this);
    imageLabel_->setFixedSize(200, 200);
    imageLabel_->setAlignment(Qt::AlignCenter);
    imageLabel_->setFrameStyle(QFrame::Box | QFrame::Sunken); // Visual improvement
    auto* pasteButton = new QPushButton("Paste Image", this);
    connect(pasteButton, &QPushButton::clicked, this, &ExerciseDialog::onPasteImage);
    layout->addWidget(new QLabel("Image (optional):", this));
    layout->addWidget(imageLabel_);
    layout->addWidget(pasteButton);

    // Description
    descriptionEdit_ = new QTextEdit(this);
    descriptionEdit_->setPlaceholderText("Enter description (max 10 lines, optional)");
    descriptionEdit_->setAcceptRichText(false);
    QFontMetrics fm(descriptionEdit_->font());
    descriptionEdit_->setMaximumHeight(10 * fm.lineSpacing() + 10); // Adjusted for padding
    layout->addWidget(new QLabel("Description:", this));
    layout->addWidget(descriptionEdit_);

    // Buttons
    auto* buttonLayout = new QHBoxLayout();
    nextButton_ = new QPushButton("Next", this);
    doneButton_ = new QPushButton("Done", this);
    connect(nextButton_, &QPushButton::clicked, this, &ExerciseDialog::onNextClicked);
    connect(doneButton_, &QPushButton::clicked, this, &ExerciseDialog::onDoneClicked);
    buttonLayout->addStretch(); // Center buttons
    buttonLayout->addWidget(nextButton_);
    buttonLayout->addWidget(doneButton_);
    layout->addLayout(buttonLayout);
}

void ExerciseDialog::onPasteImage() {
    QClipboard* clipboard = QApplication::clipboard();
    QImage image = clipboard->image();
    if (image.isNull()) {
        QMessageBox::warning(this, "Error", "No image in clipboard");
        return;
    }
    QImage scaledImage = image.scaled(200, 200, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    imageLabel_->setPixmap(QPixmap::fromImage(scaledImage));
    QBuffer buffer(&imageData_);
    buffer.open(QIODevice::WriteOnly);
    image.save(&buffer, "PNG");
    buffer.close();
}

bool ExerciseDialog::saveExercise() {
    QString name = nameEdit_->text().trimmed();
    QString description = descriptionEdit_->toPlainText().trimmed();

    if (name.isEmpty()) {
        QMessageBox::warning(this, "Error", "Name is required");
        return false;
    }

    bool success = db_.insertExercise(name.toStdString(), description.toStdString(),
                                      imageData_.isEmpty() ? nullptr : imageData_.constData(),
                                      imageData_.size());
    if (!success) {
        QMessageBox::critical(this, "Error", "Failed to save exercise to database");
    }
    return success;
}

void ExerciseDialog::onNextClicked() {
    if (saveExercise()) {
        nameEdit_->clear();
        imageLabel_->setText("No image");
        imageData_.clear();
        descriptionEdit_->clear();
        nameEdit_->setFocus(); // Improve UX
    }
}

void ExerciseDialog::onDoneClicked() {
    if (saveExercise()) {
        accept();
    }
}