#include "artwork/Pipeline.h"
#include "artwork/PipelineInternal.h"
#include "artwork/Cache.h"
#include "artwork/Placeholder.h"
#include "artwork/PixmapUtils.h"
#include "core/Paths.h"
#include "core/Game.h"
#include "providers/steam/SteamAppId.h"
#include "services/SteamGridDB.h"
#include "utils/Network.h"
#include "utils/Filesystem.h"

#include <QPixmap>
#include <QImage>
#include <QBuffer>
#include <QFile>
#include <algorithm>

using art::detail::fetchAssetByGameId;
using art::detail::fetchSteamCDN;
using art::detail::fetchSGDBBytes;
using art::detail::fromDiskCache;
using art::detail::fromSteamAppcache;
using art::detail::isSteamLocal;
using art::detail::loadPixmapBytes;
using art::detail::loadPixmapFile;
using art::detail::looksLikeImage;
using art::detail::saveToCache;
using art::detail::sgdbGameIdForSteamApp;

QPixmap art::detail::loadPixmapFile(const QString& path) {
    QPixmap p;
    if (!path.isEmpty()) p.load(path);
    return p;
}

QPixmap art::detail::loadPixmapBytes(const QByteArray& data) {
    QPixmap p;
    if (!data.isEmpty()) p.loadFromData(data);
    return p;
}

bool art::detail::looksLikeImage(const QByteArray& data) {
    if (data.size() < 4) return false;
    if (data[0] == '\x89' && data[1] == 'P') return true;
    if (static_cast<unsigned char>(data[0]) == 0xFF && static_cast<unsigned char>(data[1]) == 0xD8) return true;
    if (data.startsWith("RIFF") && data.mid(8, 4) == "WEBP") return true;
    return false;
}

bool art::detail::saveToCache(const QString& dir, const QString& key, const QByteArray& data) {
    if (!looksLikeImage(data)) return false;
    QString ext = (data[0] == '\x89') ? QStringLiteral(".png") : QStringLiteral(".jpg");
    return fs::writeBytes(dir + "/" + key + ext, data);
}

// --- Disk cache ---

QPixmap art::detail::fromDiskCache(const QString& cacheDir, const QString& key) {
    return loadPixmapFile(artcache::cachedPath(cacheDir, key));
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
