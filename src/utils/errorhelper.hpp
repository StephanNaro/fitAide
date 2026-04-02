#ifndef ERRORHELPER_HPP
#define ERRORHELPER_HPP

#include <QString>
#include <QWidget>

namespace ErrorHelper
{
    // Shows an error dialog to the user and logs to console
    void showError(QWidget* parent,
                   const QString& title,
                   const QString& message,
                   const QString& details = QString());

    // Convenience overload for database/SQL errors
    void showDbError(QWidget* parent,
                     const QString& operation,
                     const QString& sqliteErrorMsg);
}

#endif