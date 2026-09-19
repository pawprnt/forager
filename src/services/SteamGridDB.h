#pragma once
#include <QString>
#include <QByteArray>

class SteamGridDB {
public:
    static SteamGridDB& instance();
    bool isConfigured() const;
    QByteArray fetchGridsByAppId(const QString& appId);
    QByteArray fetchGridsBySearch(const QString& query);
    QByteArray fetchHeroByAppId(const QString& appId);
private:
    SteamGridDB() = default;
    QString m_token;
};
