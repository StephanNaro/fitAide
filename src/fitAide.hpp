#ifndef FITAIDE_HPP
#define FITAIDE_HPP

#include <QString>

namespace fitAide
{
#ifdef DEVELOPMENT_MODE
    const QString APP_NAME = "fitAideDev";
#else
    const QString APP_NAME = "fitAide";
#endif
    const QString DEFAULT_DB_FILENAME = APP_NAME + ".db";
}
#endif