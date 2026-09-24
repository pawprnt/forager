#pragma once

#include <QString>

namespace paths {
    QString configDir();
    QString cacheDir();
    QString cacheSub(const QString& name);
    QString gamesDir();
    QString steamAppcacheDir();
    QString protonDir();
    QString protonPrefixDir();
    QString artCacheDir();
    QString iconCacheDir();
    QString bannerCacheDir();
    QString playtimeFile();
    QString steamRoot();
    QString depotDownloaderDir();
    QString steamCmdDir();
} // namespace paths
