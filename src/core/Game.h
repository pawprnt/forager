#pragma once

#include "core/Source.h"

#include <QString>
#include <QStringList>
#include <QFileInfo>
#include <optional>

class Game {
public:
    Game() = default;
    Game(QString name, Source source);

    // Getters
    const QString& name() const { return m_name; }
    Source source() const { return m_source; }
    const QString& path() const { return m_path; }
    const QString& appId() const { return m_app_id; }
    const QStringList& launchCmd() const { return m_launch_cmd; }
    const QString& sortKey() const { return m_sort_key; }
    const QStringList& searchNames() const { return m_search_names; }
    bool isInstalled() const { return m_installed; }

    // Setters
    void setPath(const QString& path) { m_path = path; }
    void setAppId(const QString& id) { m_app_id = id; }
    void setLaunchCmd(const QStringList& cmd) { m_launch_cmd = cmd; }
    void setSortKey(const QString& key) { m_sort_key = key; }
    void setSearchNames(const QStringList& names) { m_search_names = names; }
    void setInstalled(bool installed) { m_installed = installed; }

    // Derived
    QString sourceName() const { return ::sourceName(m_source); }
    QString displayPath() const;
    QString identifier() const;

    bool operator==(const Game& other) const;
    bool operator!=(const Game& other) const { return !(*this == other); }

private:
    QString m_name;
    Source m_source = Source::Standalone;
    QString m_path;
    QString m_app_id;
    QStringList m_launch_cmd;
    QString m_sort_key;
    QStringList m_search_names;
    bool m_installed = true;
};

// For QHash / QSet
inline uint qHash(const Game& game, uint seed = 0) {
    return qHash(game.identifier(), seed);
}
