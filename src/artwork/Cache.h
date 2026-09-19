#pragma once
#include <QString>

namespace artcache {
    QString artCacheDir();
    QString iconCacheDir();
    QString bannerCacheDir();
    QString cacheKey(const QString& input);
    QString cachedPath(const QString& dir, const QString& key);
} // namespace artcache
