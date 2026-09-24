#pragma once

#include <QByteArray>
#include <QPixmap>
#include <QString>

class Game;

namespace art {
    namespace detail {
        QPixmap loadPixmapFile(const QString& path);
        QPixmap loadPixmapBytes(const QByteArray& data);
        bool looksLikeImage(const QByteArray& data);
        bool saveToCache(const QString& dir, const QString& key, const QByteArray& data);
        QString resolveSearchQuery(const Game& game);
        bool isSteamLocal(const Game& game);
        QPixmap fromSteamAppcache(const Game& game, const QString& filename);
        QPixmap fromDiskCache(const QString& cacheDir, const QString& key);
        QByteArray fetchSteamCDN(const QString& appId, const QString& filename);
        QByteArray fetchAssetByGameId(const Game& game, const char* kind, bool grid,
                                      const QString& dimensions = {});
        QString sgdbGameIdForSteamApp(const QString& appId);
        QByteArray fetchSGDBBytes(const Game& game, bool grid);
    } // namespace detail
} // namespace art
