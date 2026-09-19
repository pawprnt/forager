#pragma once

#include <QString>
#include <optional>

class SteamAppId {
public:
    static std::optional<QString> resolve(const QString& name);

private:
    static std::optional<QString> searchStore(const QString& term);
    static QString cachePath();
    static QString loadCache(const QString& key);
    static void storeCache(const QString& key, const QString& appId);
    static bool nameMatches(const QString& query, const QString& storeName);
};
