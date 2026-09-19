#include "library/Metadata.h"

bool metadata::matchesQuery(const Game& game, const QString& query)
{
    if (query.isEmpty()) return true;
    return game.name().contains(query, Qt::CaseInsensitive);
}

QString metadata::sortKey(const Game& game)
{
    QString key = game.sortKey();
    return key.isEmpty() ? game.name().toLower() : key;
}

QList<Game> metadata::filterGames(const QList<Game>& games, const QString& query)
{
    if (query.isEmpty()) return games;

    QList<Game> result;
    for (const auto& g : games) {
        if (matchesQuery(g, query)) result.append(g);
    }
    return result;
}
