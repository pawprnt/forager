#include "services/SteamGridDB.h"
#include "utils/Secrets.h"
#include "utils/Network.h"
#include <QUrl>

static const QString TOKEN_KEY = "steamgriddb_token";
static const QString BASE = "https://www.steamgriddb.com/api/v2";

SteamGridDB& SteamGridDB::instance() { static SteamGridDB s; return s; }

bool SteamGridDB::isConfigured() const { return !m_token.isEmpty(); }

QString SteamGridDB::token() const { return m_token; }

void SteamGridDB::setToken(const QString& token)
{
    m_token = token;
    if (token.isEmpty())
        secrets::clear(TOKEN_KEY);
    else
        secrets::store(TOKEN_KEY, token);
}

void SteamGridDB::loadToken()
{
    m_token = secrets::lookup(TOKEN_KEY);
}

QByteArray SteamGridDB::get(const QString& path) {
    if (!isConfigured()) return {};
    return net::httpGet(BASE + path, 10000, net::bearerHeaders(m_token));
}

QByteArray SteamGridDB::fetchGameBySteamAppId(const QString& appId) {
    return get(QStringLiteral("/games/steam/%1").arg(appId));
}

QByteArray SteamGridDB::fetchGridsByAppId(const QString& appId) {
    return get(QStringLiteral("/grids/steam/%1").arg(appId));
}

QByteArray SteamGridDB::fetchGridsByGameId(const QString& sgdbId, const QString& dimensions) {
    QString path = QStringLiteral("/grids/game/%1").arg(sgdbId);
    if (!dimensions.isEmpty())
        path += QStringLiteral("?dimensions=%1")
                    .arg(QString::fromUtf8(QUrl::toPercentEncoding(dimensions)));
    return get(path);
}

QByteArray SteamGridDB::fetchHeroesByGameId(const QString& sgdbId) {
    return get(QStringLiteral("/heroes/game/%1").arg(sgdbId));
}

QByteArray SteamGridDB::fetchHeadersByGameId(const QString& sgdbId) {
    return get(QStringLiteral("/headers/game/%1").arg(sgdbId));
}

QByteArray SteamGridDB::fetchIconsByGameId(const QString& sgdbId) {
    return get(QStringLiteral("/icons/game/%1").arg(sgdbId));
}

QByteArray SteamGridDB::fetchHeroesByAppId(const QString& appId) {
    return get(QStringLiteral("/heroes/steam/%1").arg(appId));
}

QByteArray SteamGridDB::searchAutocomplete(const QString& query) {
    return get(QStringLiteral("/search/autocomplete/%1")
        .arg(QString::fromUtf8(QUrl::toPercentEncoding(query))));
}

QByteArray SteamGridDB::fetchGridsBySearch(const QString& query) {
    return get(QStringLiteral("/search/search?terms=%1")
        .arg(QString::fromUtf8(QUrl::toPercentEncoding(query))));
}
