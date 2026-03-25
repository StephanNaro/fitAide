#include "fitAide.hpp"
#include "workoutview.hpp"
#include "exercisedialog.hpp"
#include "settingsdialog.hpp"
#include <QApplication>
#include <QtWidgets>
#include <QStringLiteral>
#include <QSettings>
#include <QFileInfo>
#include <QFileDialog>
#include <QDir>
#include <QMessageBox>
#include <iostream>

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);

    // Set application name for registry path
    QSettings settings(
        QStringLiteral("HKEY_CURRENT_USER\\Software\\%1").arg(fitAide::APP_NAME),
        QSettings::NativeFormat
    );
    QString dbPath = settings.value("DatabasePath", "").toString();

    // Check if path is valid (non-empty and file exists)
    bool validPath = false;
    if (!dbPath.isEmpty() && QFileInfo::exists(dbPath)) {
        validPath = true;
    } else {
        // Prompt user for database file location with suggested filename
        while (!validPath) {
            QFileDialog dialog(
                nullptr,
                QStringLiteral("Select Database File for %1").arg(fitAide::APP_NAME),
                QDir::homePath() + "/" + fitAide::DEFAULT_DB_FILENAME,
                QStringLiteral("Database Files (*.db);;All Files (*)")
            );
            dialog.setAcceptMode(QFileDialog::AcceptOpen);
            dialog.setFileMode(QFileDialog::AnyFile); // Allows selecting non-existent files

            if (dialog.exec() != QDialog::Accepted || dialog.selectedFiles().isEmpty()) {
                QMessageBox::question(
                    nullptr,
                    "Exit",
                    QStringLiteral("No database selected. Exit %1?").arg(fitAide::APP_NAME),
                    QMessageBox::Yes | QMessageBox::No
                );
                continue;
            }

            dbPath = dialog.selectedFiles().first();
            // Path is valid (non-empty); no existence check needed for user selection
            validPath = true;
            // Save valid path to registry
            settings.setValue("DatabasePath", dbPath);
        }
    }

    // Initialize database with the selected path
    Database db(dbPath.toStdString());
    if (!db.initialize()) {
        std::cerr << "Failed to initialize database" << std::endl;
        return 1;
    }

    if (!db.hasExercises()) {
        ExerciseDialog dialog(db);
        if (dialog.exec() == QDialog::Rejected) return 0;
    }

    if (!db.hasSettings()) {
        SettingsDialog dialog(db);
        if (dialog.exec() == QDialog::Rejected) {
            std::cout << "Settings dialog cancelled, exiting..." << std::endl;
            return 0;
        }
    }

    WorkoutView workoutView(db);
    workoutView.show();
    return app.exec();
}