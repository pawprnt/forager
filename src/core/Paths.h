#pragma once

#include <QString>
#include <QDir>

namespace paths {
    QString configDir();
    QString cacheDir();
    QString gamesDir();
    QString steamAppcacheDir();
    QString protonDir();
    QString protonPrefixDir();
    QString artCacheDir();
    QString iconCacheDir();
    QString bannerCacheDir();
    QString playtimeFile();
} // namespace paths
