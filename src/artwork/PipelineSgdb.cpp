#include "artwork/PipelineInternal.h"
#include "core/Game.h"
#include "providers/steam/SteamAppId.h"
#include "services/SteamGridDB.h"
#include "utils/Network.h"
#include "utils/Json.h"

#include <QJsonObject>
#include <QJsonArray>

// --- SteamGridDB helpers ---

static int gridRank(const QJsonObject& item) {
    int w = item["width"].toInt();
    int h = item["height"].toInt();
    int score = item["score"].toInt();
    QString style = item["style"].toString();
    int rank = score;
    if (style == "official") rank += 100000;
    if (w == 600 && h == 900) rank += 50000;
    return rank;
}

static int heroRank(const QJsonObject& item) {
    int score = item["score"].toInt();
    int rank = score;
    if (item["width"].toInt() >= 460) rank += 50000;
    return rank;
}

static QString pickBestUrl(const QByteArray& payload, int (*rankFn)(const QJsonObject&)) {
    auto obj = json::parseObject(payload);
    if (!obj) return {};
    QJsonArray data = (*obj)["data"].toArray();
    if (data.isEmpty()) return {};

    int bestIdx = 0;
    int bestScore = -1;
    for (int i = 0; i < data.size(); ++i) {
        int rank = rankFn(data[i].toObject());
        if (rank > bestScore) {
            bestScore = rank;
            bestIdx = i;
        }
    }
    return data[bestIdx].toObject()["url"].toString();
}

static QString matchAutocompleteEntry(const QByteArray& payload, const QString& query) {
    auto obj = json::parseObject(payload);
    if (!obj) return {};
    QJsonArray data = (*obj)["data"].toArray();
    if (data.isEmpty()) return {};

    QString q = query.toLower();
    for (const auto& v : data) {
        QJsonObject entry = v.toObject();
        QString name = entry["name"].toString().toLower();
        if (name.contains(q) || (q.length() >= 6 && name.startsWith(q)))
            return QString::number(entry["id"].toInt());
    }
    if (!data.isEmpty())
        return QString::number(data[0].toObject()["id"].toInt());
    return {};
}

QString art::detail::resolveSearchQuery(const Game& game) {
    if (!game.searchNames().isEmpty()) return game.searchNames().first();
    QString name = game.name();
    int slash = name.lastIndexOf('/');
    if (slash >= 0) name = name.mid(slash + 1);
    return name;
}

QString art::detail::sgdbGameIdForSteamApp(const QString& appId) {
    auto& sgdb = SteamGridDB::instance();
    if (!sgdb.isConfigured() || appId.isEmpty()) return {};
    auto obj = json::parseObject(sgdb.fetchGameBySteamAppId(appId));
    if (!obj) return {};
    QJsonValue data = (*obj)["data"];
    if (data.isObject()) {
        int id = data.toObject()["id"].toInt();
        if (id > 0) return QString::number(id);
    }
    return {};
}

QByteArray art::detail::fetchAssetByGameId(const Game& game, const char* kind, bool grid,
                                 const QString& dimensions) {
    auto& sgdb = SteamGridDB::instance();
    if (!sgdb.isConfigured()) return {};

    auto plan = game.sgdbSearch();
    if (!plan) return {};
    const auto& [queries, matchTerm] = *plan;
    Q_UNUSED(matchTerm);

    int (*rankFn)(const QJsonObject&) = grid ? gridRank : heroRank;

    for (const QString& q : queries) {
        QByteArray searchResp = sgdb.searchAutocomplete(q);
        QString sgdbId = matchAutocompleteEntry(searchResp, q);
        if (sgdbId.isEmpty()) continue;

        QByteArray resp;
        if (qstrcmp(kind, "grids") == 0)
            resp = sgdb.fetchGridsByGameId(sgdbId, dimensions);
        else if (qstrcmp(kind, "headers") == 0)
            resp = sgdb.fetchHeadersByGameId(sgdbId);
        else if (qstrcmp(kind, "heroes") == 0)
            resp = sgdb.fetchHeroesByGameId(sgdbId);
        else if (qstrcmp(kind, "icons") == 0)
            resp = sgdb.fetchIconsByGameId(sgdbId);
        else
            continue;

        auto obj = json::parseObject(resp);
        if (!obj || (*obj)["data"].toArray().isEmpty()) continue;

        QString url = pickBestUrl(resp, rankFn);
        if (url.isEmpty()) continue;
        QByteArray data = net::httpGet(url, 15000);
        if (!data.isEmpty()) return data;
    }
    return {};
}

QByteArray art::detail::fetchSGDBBytes(const Game& game, bool grid) {
    auto& sgdb = SteamGridDB::instance();
    if (!sgdb.isConfigured()) return {};

    QString appId = SteamAppId::resolve(game).value_or(QString());
    int (*rankFn)(const QJsonObject&) = grid ? gridRank : heroRank;

    // 1. Steam app ID path (Steam games or confidently resolved)
    if (!appId.isEmpty()) {
        if (grid) {
            QByteArray resp = sgdb.fetchGridsByAppId(appId);
            auto obj = json::parseObject(resp);
            if (obj && !(*obj)["data"].toArray().isEmpty()) {
                QString url = pickBestUrl(resp, rankFn);
                if (!url.isEmpty()) {
                    QByteArray data = net::httpGet(url, 15000);
                    if (!data.isEmpty()) return data;
                }
            }
        } else {
            QString sgdbId = sgdbGameIdForSteamApp(appId);
            if (!sgdbId.isEmpty()) {
                QByteArray resp = sgdb.fetchHeroesByGameId(sgdbId);
                auto obj = json::parseObject(resp);
                if (obj && !(*obj)["data"].toArray().isEmpty()) {
                    QString url = pickBestUrl(resp, rankFn);
                    if (!url.isEmpty()) {
                        QByteArray data = net::httpGet(url, 15000);
                        if (!data.isEmpty()) return data;
                    }
                }
            }
        }
    }

    // 2. Name/path-based search for non-Steam (or fallback)
    if (game.source() != Source::Steam) {
        QByteArray data = fetchAssetByGameId(game, grid ? "grids" : "heroes", grid);
        if (!data.isEmpty()) return data;
    }

    // 3. Leaf-name autocomplete fallback
    QString query = resolveSearchQuery(game);
    if (query.isEmpty()) return {};

    QByteArray searchResp = sgdb.searchAutocomplete(query);
    QString sgdbId = matchAutocompleteEntry(searchResp, query);
    if (sgdbId.isEmpty()) return {};

    QByteArray resp = grid ? sgdb.fetchGridsByGameId(sgdbId)
                           : sgdb.fetchHeroesByGameId(sgdbId);
    auto obj = json::parseObject(resp);
    if (!obj || (*obj)["data"].toArray().isEmpty()) return {};

    QString url = pickBestUrl(resp, rankFn);
    if (url.isEmpty()) return {};
    return net::httpGet(url, 15000);
}
