#ifndef FITAIDE_HPP
#define FITAIDE_HPP

#include "database.hpp"
#include <QString>
#include <QByteArray>
#include <vector>
#include <string>

namespace fitAide {
#ifdef DEVELOPMENT_MODE
    inline constexpr const char* APP_NAME = "fitAideDev";
#else
    inline constexpr const char* APP_NAME = "fitAide";
#endif
    inline constexpr const char* DEFAULT_DB_FILENAME = "fitAide.db";
}
#endif