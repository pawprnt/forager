#pragma once

#include "core/Game.h"
#include <QString>
#include <optional>

class SteamAppId {
public:
    static std::optional<QString> resolve(const Game& game);
    static std::optional<QString> resolveByName(const QString& name);

private:
    static QStringList searchTerms(const Game& game);
    static std::optional<QString> searchStore(const QString& term);
    static bool nameMatches(const QString& query, const QString& storeName);
};
