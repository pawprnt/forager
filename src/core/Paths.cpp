#include "core/Paths.h"
#include "core/Config.h"
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

QString cacheSub(const QString& name)
{
    return cacheDir() + "/" + name;
}

QString gamesDir()
{
    return Config::instance().gamesDir();
}

QString steamAppcacheDir()
{
    return Config::instance().steamAppcache();
}

QString protonDir() { return cacheSub("proton"); }
QString protonPrefixDir() { return cacheSub("proton-prefix"); }
QString artCacheDir() { return cacheSub("art"); }
QString iconCacheDir() { return cacheSub("icons"); }
QString bannerCacheDir() { return cacheSub("banners"); }

QString playtimeFile()
{
    return configDir() + "/playtime.json";
}

QString steamRoot() { return QDir::homePath() + "/.local/share/Steam"; }
QString depotDownloaderDir() { return cacheSub("depotdownloader"); }
QString steamCmdDir() { return cacheSub("steamcmd"); }

} // namespace paths
