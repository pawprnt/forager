#pragma once

#include <QString>
#include <QJsonObject>

class Config {
public:
    static Config& instance();

    void load();
    void save();

    QString gamesDir() const;
    QString steamAppcache() const;
    QString displaySize() const;
    bool protonFeature(const QString& name) const;

    void setGamesDir(const QString& dir);
    void setSteamAppcache(const QString& path);
    void setDisplaySize(const QString& size);

    const QJsonObject& data() const { return m_data; }

private:
    Config() = default;
    QJsonObject m_data;
    QString m_path;

    static QJsonObject deepMerge(const QJsonObject& base, const QJsonObject& override_);
};
