#pragma once

#include "core/Game.h"

#include <QObject>
#include <QJsonObject>
#include <QMutex>
#include <QProcess>
#include <QTimer>
#include <QMap>
#include <functional>

class PlaytimeStore : public QObject {
    Q_OBJECT

public:
    explicit PlaytimeStore(const QString& path = {}, QObject* parent = nullptr);

    void load();
    void save();

    float playtime(const QString& key) const;
    float lastPlayed(const QString& key) const;
    void touch(const QString& key);
    void add(const QString& key, float seconds);

    static QString gameKey(const Game& game);
    static QString formatPlaytime(float seconds);

private:
    void mutateEntry(const QString& key, const std::function<void(QJsonObject&)>& fn);

    QString m_path;
    QJsonObject m_data;
    mutable QMutex m_mutex;
};

class PlaytimeTracker : public QObject {
    Q_OBJECT

public:
    explicit PlaytimeTracker(PlaytimeStore* store, QObject* parent = nullptr);

    PlaytimeStore* store() const { return m_store; }
    bool hasSessions() const;

    void begin(const Game& game, QProcess* proc);
    bool tick();
    void flush();
    bool isRunning(const Game& game) const;
    bool stop(const Game& game);
    QList<Game> recentlyPlayed(const QList<Game>& games, int limit = 8) const;

private:
    struct Session {
        QProcess* proc = nullptr;
        float last = 0.0f;
    };

    bool flushSession(const QString& key, Session& sess, float now);

    PlaytimeStore* m_store;
    QMap<QString, Session> m_sessions;
};
