#include "fitAide.hpp"
#include "utils/errorhelper.hpp"
#include "database/database.hpp"
#include "views/workoutview.hpp"
#include "dialogs/exercisedialog.hpp"
#include "dialogs/settingsdialog.hpp"
#include <QApplication>
#include <QtWidgets>
#include <QStringLiteral>
#include <QSettings>
#include <QFileInfo>
#include <QFileDialog>
#include <QDir>
#include <QMessageBox>
#include <iostream>

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    app.setOrganizationName("fitAide");
    app.setApplicationName(fitAide::APP_NAME);

    QSettings settings(QSettings::UserScope, "fitAide", fitAide::APP_NAME);

    QString dbPath = settings.value("DatabasePath", "").toString();

    // Check if path is valid (non-empty and file exists)
    bool validPath = false;
    if (!dbPath.isEmpty() && QFileInfo::exists(dbPath))
    {
        validPath = true;
    }
    else {
        while (!validPath) {
            QFileDialog dialog(
                nullptr,
                QStringLiteral("Select Database File for %1").arg(fitAide::APP_NAME),
                QDir::homePath() + "/" + fitAide::DEFAULT_DB_FILENAME,
                QStringLiteral("Database Files (*.db);;All Files (*)")
            );
            dialog.setAcceptMode(QFileDialog::AcceptOpen);
            dialog.setFileMode(QFileDialog::AnyFile); // Allows selecting non-existent files

            if (dialog.exec() != QDialog::Accepted || dialog.selectedFiles().isEmpty())
            {
                auto reply = QMessageBox::question(
                    nullptr, "Exit",
                    QStringLiteral("No database selected. Exit %1?").arg(fitAide::APP_NAME),
                    QMessageBox::Yes | QMessageBox::No);

                if (reply == QMessageBox::Yes)
                    return 0;

                continue;
            }

            dbPath = dialog.selectedFiles().first();
            validPath = true;
            settings.setValue("DatabasePath", dbPath);
        }
    }

    // Initialize database with the selected path
    Database db(dbPath.toStdString());
    if (!db.initialize())
    {
        ErrorHelper::showError(nullptr, "Initialization Error",
                               "Failed to initialize database schema.");
        return 1;
    }

    if (!db.hasExercises())
    {
        ExerciseDialog dialog(db);
        if (dialog.exec() == QDialog::Rejected) return 0;
    }

    if (!db.hasSettings())
    {
        SettingsDialog dialog(db);
        if (dialog.exec() == QDialog::Rejected)
        {
            std::cout << "Settings dialog cancelled, exiting..." << std::endl;
            return 0;
        }
    }

    if (db.isCooldownActive())
    {
        QString lastTime = db.getLastWorkoutTime();
        QDateTime lastDT = QDateTime::fromString(lastTime, "yyyy-MM-dd HH:mm:ss");

        QString msg = QString("Your last workout was on %1.\n\n"
                              "Only %2 hours have passed since then.\n\n"
                              "It is recommended to wait at least 48 hours between workouts "
                              "for the same muscle groups.\n\n"
                              "Do you want to proceed anyway?")
                          .arg(lastDT.toString("yyyy-MM-dd HH:mm"))
                          .arg(lastDT.secsTo(QDateTime::currentDateTime()) / 3600);

        auto reply = QMessageBox::warning(nullptr, "Cooldown Reminder",
                                          msg,
                                          QMessageBox::Yes | QMessageBox::No,
                                          QMessageBox::No);

        if (reply == QMessageBox::No)
            return 0;   // exit app
    }

    WorkoutView workoutView(db);
    workoutView.show();
    return app.exec();
}