#include "artwork/Cache.h"
#include "core/Paths.h"
#include <QDir>
#include <QCryptographicHash>

QString artcache::artCacheDir() { return paths::artCacheDir(); }
QString artcache::iconCacheDir() { return paths::iconCacheDir(); }
QString artcache::bannerCacheDir() { return paths::bannerCacheDir(); }

QString artcache::cacheKey(const QString& input) {
    return QCryptographicHash::hash(input.toUtf8(), QCryptographicHash::Sha256).left(16).toHex();
}

QString artcache::cachedPath(const QString& dir, const QString& key) {
    QDir d(dir);
    for (const auto& ext : {"*.jpg", "*.png"}) {
        QStringList files = d.entryList({key + ext}, QDir::Files);
        if (!files.isEmpty()) return d.absoluteFilePath(files.first());
    }
    return {};
}
