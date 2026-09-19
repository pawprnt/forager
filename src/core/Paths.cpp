#include "core/Paths.h"
#include "app/Constants.h"

#include <QDir>
#include <QStandardPaths>
#include <QCoreApplication>

namespace paths {

static QString overrideOr(const QString& envVar, const QString& fallback)
{
    QByteArray val = qgetenv(envVar.toUtf8().constData());
    return val.isEmpty() ? fallback : QString::fromLocal8Bit(val);
}

QString configDir()
{
    return overrideOr("FORAGER_CONFIG_DIR",
        QStandardPaths::writableLocation(QStandardPaths::ConfigLocation)
            + "/" + constants::APP_NAME);
}

QString cacheDir()
{
    return overrideOr("FORAGER_CACHE_DIR",
        QStandardPaths::writableLocation(QStandardPaths::CacheLocation));
}

QString gamesDir()
{
    return QDir::homePath() + "/Games";
}

QString steamAppcacheDir()
{
    return QDir::homePath()
        + "/.local/share/Steam/appcache/librarycache";
}

QString protonDir()
{
    return cacheDir() + "/proton";
}

QString protonPrefixDir()
{
    return cacheDir() + "/proton-prefix";
}

QString artCacheDir()
{
    return cacheDir() + "/art";
}

QString iconCacheDir()
{
    return cacheDir() + "/icons";
}

QString bannerCacheDir()
{
    return cacheDir() + "/banners";
}

QString playtimeFile()
{
    return configDir() + "/playtime.json";
}

} // namespace paths
