#pragma once

#include "core/Game.h"
#include <QList>

namespace metadata {
    bool matchesQuery(const Game& game, const QString& query);
    QString sortKey(const Game& game);
    QList<Game> filterGames(const QList<Game>& games, const QString& query);
} // namespace metadata
