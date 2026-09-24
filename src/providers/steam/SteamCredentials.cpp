#include "providers/steam/SteamCredentials.h"
#include "core/Paths.h"
#include "utils/Json.h"

#include <QFile>
#include <QJsonObject>

SteamCredentials& SteamCredentials::instance()
{
    static SteamCredentials s_instance;
    return s_instance;
}

SteamCredentials::SteamCredentials()
    : m_path(paths::configDir() + "/steam_credentials.json")
{
    load();
}

bool SteamCredentials::hasCredentials() const
{
    return !m_username.isEmpty() && !m_steamId.isEmpty();
}

void SteamCredentials::setUsername(const QString& username)
{
    m_username = username;
    save();
}

void SteamCredentials::setSteamId(const QString& steamId)
{
    m_steamId = steamId;
    save();
}

void SteamCredentials::setWebApiKey(const QString& apiKey)
{
    m_webApiKey = apiKey;
    save();
}

void SteamCredentials::setLoginSecure(const QString& cookie)
{
    m_loginSecure = cookie;
    save();
}

void SteamCredentials::setAll(const QString& username, const QString& steamId,
                              const QString& apiKey, const QString& cookie)
{
    m_username = username;
    m_steamId = steamId;
    m_webApiKey = apiKey;
    m_loginSecure = cookie;
    save();
}

void SteamCredentials::clear()
{
    m_username.clear();
    m_steamId.clear();
    m_webApiKey.clear();
    m_loginSecure.clear();
    QFile::remove(m_path);
}

void SteamCredentials::load()
{
    auto data = json::readObject(m_path);
    if (!data) return;

    m_username = (*data)["username"].toString();
    m_steamId = (*data)["steam_id"].toString();
    m_webApiKey = (*data)["web_api_key"].toString();
    m_loginSecure = (*data)["login_secure"].toString();
}

void SteamCredentials::save()
{
    QJsonObject o;
    o["username"] = m_username;
    o["steam_id"] = m_steamId;
    o["web_api_key"] = m_webApiKey;
    o["login_secure"] = m_loginSecure;

    json::writeObject(m_path, o);
}
