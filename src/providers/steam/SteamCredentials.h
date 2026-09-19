#pragma once

#include <QString>

class SteamCredentials {
public:
    static SteamCredentials& instance();

    QString username() const { return m_username; }
    QString steamId() const { return m_steamId; }
    QString webApiKey() const { return m_webApiKey; }
    QString loginSecure() const { return m_loginSecure; }

    bool hasCredentials() const;

    void setUsername(const QString& username);
    void setSteamId(const QString& steamId);
    void setWebApiKey(const QString& apiKey);
    void setLoginSecure(const QString& cookie);
    void setAll(const QString& username, const QString& steamId,
                const QString& apiKey, const QString& cookie);

    void clear();

private:
    SteamCredentials();
    void load();
    void save();

    QString m_username;
    QString m_steamId;
    QString m_webApiKey;
    QString m_loginSecure;
    QString m_path;
};
