#include "artwork/Pipeline.h"
#include "artwork/Cache.h"
#include "artwork/Placeholder.h"
#include "artwork/PixmapUtils.h"
#include "core/Paths.h"
#include "core/Game.h"
#include "providers/steam/SteamAppId.h"
#include "services/SteamGridDB.h"
#include "utils/Network.h"
#include "utils/Json.h"
#include "utils/Filesystem.h"

#include <QPixmap>
#include <QJsonObject>
#include <QJsonArray>
#include <QImage>
#include <QBuffer>
#include <QFile>
#include <algorithm>
#include <QFile>
#include <algorithm>
#include <QFile>
#include <algorithm>

static const char* STEAM_CDN = "https://shared.akamai.steamstatic.com/store_item_assets/steam/apps";

static QPixmap loadPixmapFile(const QString& path) {
    QPixmap p;
    if (!path.isEmpty()) p.load(path);
    return p;
}

static QPixmap loadPixmapBytes(const QByteArray& data) {
    QPixmap p;
    if (!data.isEmpty()) p.loadFromData(data);
    return p;
}

static bool looksLikeImage(const QByteArray& data) {
    if (data.size() < 4) return false;
    if (data[0] == '\x89' && data[1] == 'P') return true;
    if (static_cast<unsigned char>(data[0]) == 0xFF && static_cast<unsigned char>(data[1]) == 0xD8) return true;
    if (data.startsWith("RIFF") && data.mid(8, 4) == "WEBP") return true;
    return false;
}

static bool saveToCache(const QString& dir, const QString& key, const QByteArray& data) {
    if (!looksLikeImage(data)) return false;
    QString ext = (data[0] == '\x89') ? QStringLiteral(".png") : QStringLiteral(".jpg");
    return fs::writeBytes(dir + "/" + key + ext, data);
}

static QString resolveSearchQuery(const Game& game) {
    if (!game.searchNames().isEmpty()) return game.searchNames().first();
    QString name = game.name();
    int slash = name.lastIndexOf('/');
    if (slash >= 0) name = name.mid(slash + 1);
    return name;
}

static bool isSteamLocal(const Game& game) {
    return game.source() == Source::Steam && !game.appId().isEmpty();
}

// --- Steam appcache ---

static QPixmap fromSteamAppcache(const Game& game, const QString& filename) {
    if (!isSteamLocal(game)) return {};
    QString dir = paths::steamAppcacheDir();
    if (dir.isEmpty()) return {};
    return loadPixmapFile(dir + "/" + game.appId() + "/" + filename);
}

// --- Disk cache ---

static QPixmap fromDiskCache(const QString& cacheDir, const QString& key) {
    return loadPixmapFile(artcache::cachedPath(cacheDir, key));
}

// --- Steam CDN ---

static QByteArray fetchSteamCDN(const QString& appId, const QString& filename) {
    if (appId.isEmpty()) return {};
    QString url = QStringLiteral("%1/%2/%3").arg(STEAM_CDN, appId, filename);
    return net::httpGet(url, 10000);
}

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

static QString sgdbGameIdForSteamApp(const QString& appId) {
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

static QByteArray fetchAssetByGameId(const Game& game, const char* kind, bool grid,
                                     const QString& dimensions = {}) {
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

static QByteArray fetchSGDBBytes(const Game& game, bool grid) {
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

// --- Unified load pipeline ---

static QPixmap loadArt(const Game& game, bool allowNetwork, bool isGrid) {
    QString key = artcache::cacheKey(game.identifier());
    static const char* gridFiles[] = {"library_600x900.jpg", "library_600x900_2x.jpg", "header.jpg"};
    static const char* heroFiles[] = {"library_hero.jpg", "library_hero_blur.jpg", "header.jpg"};
    const char** cdnFiles = isGrid ? gridFiles : heroFiles;
    int cdnCount = isGrid ? 3 : 3;
    const QString& cacheDir = isGrid ? paths::artCacheDir() : paths::bannerCacheDir();
    const QString cachePrefix = isGrid ? QStringLiteral("grid_") : QStringLiteral("hero_");
    const QString cacheKey = cachePrefix + key;

    // 1. Local steam appcache (Steam only)
    QPixmap p;
    for (int i = 0; i < cdnCount && p.isNull(); ++i)
        p = fromSteamAppcache(game, cdnFiles[i]);
    if (!p.isNull()) return p;

    // 2. Disk cache
    p = fromDiskCache(cacheDir, cacheKey);
    if (!p.isNull()) return p;

    if (!allowNetwork) {
        if (isGrid)
            return placeholder::placeholderGrid(game.name(), 165, 248);
        return p;
    }

    // 3. Steam CDN via resolved app id
    QString appId = SteamAppId::resolve(game).value_or(QString());
    QByteArray cdn;
    for (int i = 0; i < cdnCount && cdn.isEmpty(); ++i)
        cdn = fetchSteamCDN(appId, cdnFiles[i]);
    if (!cdn.isEmpty()) {
        saveToCache(cacheDir, cacheKey, cdn);
        p = loadPixmapBytes(cdn);
        if (!p.isNull()) return p;
    }

    // 4. SteamGridDB
    QByteArray sgdbData = fetchSGDBBytes(game, isGrid);
    if (!sgdbData.isEmpty()) {
        saveToCache(cacheDir, cacheKey, sgdbData);
        p = loadPixmapBytes(sgdbData);
        if (!p.isNull()) return p;
    }

    // 5. Hero falls back to header
    if (!isGrid && !appId.isEmpty()) {
        QByteArray headerData = fetchSteamCDN(appId, "header.jpg");
        if (!headerData.isEmpty()) {
            saveToCache(cacheDir, cacheKey, headerData);
            p = loadPixmapBytes(headerData);
            return p;
        }
    }

    if (isGrid)
        return placeholder::placeholderGrid(game.name(), 165, 248);
    return p;
}

// --- Public API ---

QPixmap art::loadGrid(const Game& game, bool allowNetwork) {
    return loadArt(game, allowNetwork, true);
}

QPixmap art::loadHero(const Game& game, bool allowNetwork) {
    return loadArt(game, allowNetwork, false);
}

QPixmap art::loadIcon(const Game& game, bool allowNetwork) {
    const QString& iconDir = paths::iconCacheDir();
    QString key = artcache::cacheKey(game.identifier());
    QString cached = artcache::cachedPath(iconDir, key);
    if (!cached.isEmpty()) {
        QPixmap p = loadPixmapFile(cached);
        if (!p.isNull()) return p.scaled(48, 48, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }

    // Local steam logo (square crop)
    if (isSteamLocal(game)) {
        QString logoPath = paths::steamAppcacheDir() + "/" + game.appId() + "/logo.png";
        QImage img(logoPath);
        if (!img.isNull()) {
            int size = std::min(img.width(), img.height());
            if (size > 0) {
                int ox = (img.width() - size) / 2;
                int oy = (img.height() - size) / 2;
                QImage square = img.copy(ox, oy, size, size);
                QPixmap pix = QPixmap::fromImage(square).scaled(
                    48, 48, Qt::KeepAspectRatio, Qt::SmoothTransformation);
                if (!pix.isNull()) {
                    QByteArray png;
                    QBuffer buf(&png);
                    buf.open(QIODevice::WriteOnly);
                    if (pix.save(&buf, "PNG"))
                        fs::writeBytes(iconDir + "/" + key + ".png", png);
                    return pix;
                }
            }
        }
    }

    // Local icon files next to the game
    if (!game.path().isEmpty()) {
        for (const char* name : {"icon.png", "icon.ico", "icon.svg", "Icon.png", "Icon.ico"}) {
            QString candidate = game.path() + "/" + name;
            if (QFile::exists(candidate)) {
                QPixmap pix(candidate);
                if (!pix.isNull())
                    return pix.scaled(48, 48, Qt::KeepAspectRatio, Qt::SmoothTransformation);
            }
        }
        QString mcIcon = game.path() + "/.minecraft/icon.png";
        if (QFile::exists(mcIcon)) {
            QPixmap pix(mcIcon);
            if (!pix.isNull())
                return pix.scaled(48, 48, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        }
    }

    if (!allowNetwork) return {};

    auto& sgdb = SteamGridDB::instance();
    if (sgdb.isConfigured()) {
        QByteArray data;
        QString appId = isSteamLocal(game) ? game.appId()
                                           : SteamAppId::resolve(game).value_or(QString());
        if (game.source() == Source::Steam && !appId.isEmpty()) {
            QString sgdbId = sgdbGameIdForSteamApp(appId);
            if (!sgdbId.isEmpty())
                data = net::httpGet(
                    QStringLiteral("https://www.steamgriddb.com/api/v2/icons/game/%1").arg(sgdbId),
                    15000, net::bearerHeaders(sgdb.token()));
        }
        if (data.isEmpty() && game.source() != Source::Steam)
            data = fetchAssetByGameId(game, "icons", false);
        if (!data.isEmpty() && looksLikeImage(data)) {
            fs::writeBytes(iconDir + "/" + key + ".png", data);
            return loadPixmapBytes(data).scaled(48, 48, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        }
    }
    return {};
}

QByteArray art::loadGridBytes(const Game& game) {
    return pixmap::toJpeg(loadGrid(game, false));
}

QByteArray art::loadHeroBytes(const Game& game) {
    return pixmap::toJpeg(loadHero(game, false));
}

void art::registerPlaceholderFont() {}
