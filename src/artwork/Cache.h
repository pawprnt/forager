#pragma once
#include <QString>

namespace artcache {
    QString cacheKey(const QString& input);
    QString cachedPath(const QString& dir, const QString& key);
} // namespace artcache
