#pragma once
#include <QString>
#include <QByteArray>

class SteamGridDB {
public:
    static SteamGridDB& instance();
    bool isConfigured() const;
    QString token() const;
    void setToken(const QString& token);
    void loadToken();
    QByteArray fetchGameBySteamAppId(const QString& appId);
    QByteArray fetchGridsByAppId(const QString& appId);
    QByteArray fetchGridsByGameId(const QString& sgdbId, const QString& dimensions = {});
    QByteArray fetchHeroesByGameId(const QString& sgdbId);
    QByteArray fetchHeadersByGameId(const QString& sgdbId);
    QByteArray fetchIconsByGameId(const QString& sgdbId);
    QByteArray fetchHeroesByAppId(const QString& appId);
    QByteArray searchAutocomplete(const QString& query);
    QByteArray fetchGridsBySearch(const QString& query);
private:
    QByteArray get(const QString& path);
    SteamGridDB() = default;
    QString m_token;
};
