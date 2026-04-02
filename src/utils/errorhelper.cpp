#include "errorhelper.hpp"
#include <QMessageBox>
#include <iostream>

namespace ErrorHelper
{
    void showError(QWidget* parent,
                   const QString& title,
                   const QString& message,
                   const QString& details)
    {
        std::cerr << "ERROR: " << title.toStdString() << " - " << message.toStdString();
        if (!details.isEmpty())
            std::cerr << " (" << details.toStdString() << ")";
        std::cerr << std::endl;

        QMessageBox box(parent);
        box.setIcon(QMessageBox::Critical);
        box.setWindowTitle(title);
        box.setText(message);

        if (!details.isEmpty())
            box.setInformativeText(details);

        box.exec();
    }

    void showDbError(QWidget* parent,
                     const QString& operation,
                     const QString& sqliteErrorMsg)
    {
        QString msg = QString("Failed to %1.").arg(operation);
        QString details = QString("SQLite error: %1").arg(sqliteErrorMsg);

        showError(parent, "Database Error", msg, details);
    }
}