#ifndef EXERCISEINSTRUCT_HPP
#define EXERCISEINSTRUCT_HPP

#include <QWidget>
#include <QString>
#include <QByteArray>
#include <QLabel>

class ExerciseInstruct : public QWidget
{
    Q_OBJECT
public:
    explicit ExerciseInstruct(const QString& description, const QByteArray& imageData, QWidget* parent = nullptr);
    void setDescription(const QString& description);
    void setImage(const QByteArray& imageData);

private:
    QLabel* descriptionLabel_;
    QLabel* imageLabel_;
};

#endif