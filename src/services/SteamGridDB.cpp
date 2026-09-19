#include "services/SteamGridDB.h"
#include "utils/Network.h"
#include <QJsonDocument>
#include <QJsonObject>

SteamGridDB& SteamGridDB::instance() { static SteamGridDB s; return s; }

bool SteamGridDB::isConfigured() const { return !m_token.isEmpty(); }

QByteArray SteamGridDB::fetchGridsByAppId(const QString& appId) {
    if (!isConfigured()) return {};
    QString url = QStringLiteral("https://www.steamgriddb.com/api/v2/grids/steam/%1").arg(appId);
    return net::httpGet(url, 10000, {{"Authorization", "Bearer " + m_token}});
}

QByteArray SteamGridDB::fetchGridsBySearch(const QString& query) {
    if (!isConfigured()) return {};
    QString url = QStringLiteral("https://www.steamgriddb.com/api/v2/search/search?terms=%1").arg(query);
    return net::httpGet(url, 10000, {{"Authorization", "Bearer " + m_token}});
}

QByteArray SteamGridDB::fetchHeroByAppId(const QString& appId) {
    if (!isConfigured()) return {};
    QString url = QStringLiteral("https://www.steamgriddb.com/api/v2/heroes/steam/%1").arg(appId);
    return net::httpGet(url, 10000, {{"Authorization", "Bearer " + m_token}});
}
