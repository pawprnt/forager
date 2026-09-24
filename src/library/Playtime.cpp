#include "library/Playtime.h"
#include "utils/Json.h"
#include "core/Paths.h"

#include <QJsonObject>
#include <QElapsedTimer>

static float nowSeconds()
{
    static QElapsedTimer timer;
    if (!timer.isValid()) timer.start();
    return timer.elapsed() / 1000.0f;
}

// --- PlaytimeStore ---

PlaytimeStore::PlaytimeStore(const QString& path, QObject* parent)
    : QObject(parent)
    , m_path(path.isEmpty() ? paths::playtimeFile() : path)
{
    load();
}

void PlaytimeStore::load()
{
    auto data = json::readObject(m_path);
    if (!data) return;

    QMutexLocker lock(&m_mutex);
    m_data = *data;
}

void PlaytimeStore::save()
{
    QJsonObject snapshot;
    {
        QMutexLocker lock(&m_mutex);
        snapshot = m_data;
    }

    json::writeObject(m_path, snapshot);
}

float PlaytimeStore::playtime(const QString& key) const
{
    QMutexLocker lock(&m_mutex);
    return m_data[key].toObject()["playtime"].toDouble(0.0);
}

float PlaytimeStore::lastPlayed(const QString& key) const
{
    QMutexLocker lock(&m_mutex);
    return m_data[key].toObject()["last_played"].toDouble(0.0);
}

void PlaytimeStore::mutateEntry(const QString& key, const std::function<void(QJsonObject&)>& fn)
{
    QMutexLocker lock(&m_mutex);
    QJsonObject entry = m_data[key].toObject();
    fn(entry);
    m_data[key] = entry;
}

void PlaytimeStore::touch(const QString& key)
{
    mutateEntry(key, [](QJsonObject& entry) {
        entry["last_played"] = nowSeconds();
        if (!entry.contains("playtime")) entry["playtime"] = 0.0;
    });
}

void PlaytimeStore::add(const QString& key, float seconds)
{
    if (seconds <= 0) return;
    mutateEntry(key, [seconds](QJsonObject& entry) {
        entry["playtime"] = entry["playtime"].toDouble(0.0) + seconds;
        if (!entry.contains("last_played")) entry["last_played"] = 0.0;
    });
}

QString PlaytimeStore::gameKey(const Game& game)
{
    if (!game.appId().isEmpty())
        return "steam:" + game.appId();

    QString path = game.path();
    if (!path.isEmpty()) {
        // Canonical path would be better, but may not exist at lookup time
        return game.sourceName().toLower() + ":" + path;
    }
    return game.sourceName().toLower() + ":" + game.name();
}

QString PlaytimeStore::formatPlaytime(float seconds)
{
    if (seconds < 60)
        return QStringLiteral("%1 s").arg(static_cast<int>(seconds));
    if (seconds < 3600)
        return QStringLiteral("%1 min").arg(static_cast<int>(seconds / 60));
    return QStringLiteral("%1 h").arg(seconds / 3600.0f, 0, 'f', 1);
}

// --- PlaytimeTracker ---

PlaytimeTracker::PlaytimeTracker(PlaytimeStore* store, QObject* parent)
    : QObject(parent)
    , m_store(store)
{
}

bool PlaytimeTracker::hasSessions() const
{
    return !m_sessions.isEmpty();
}

void PlaytimeTracker::begin(const Game& game, QProcess* proc)
{
    QString key = PlaytimeStore::gameKey(game);
    m_store->touch(key);
    if (proc) {
        m_sessions[key] = {proc, nowSeconds()};
    }
    m_store->save();
}

bool PlaytimeTracker::tick()
{
    float current = nowSeconds();
    bool dirty = false;

    auto it = m_sessions.begin();
    while (it != m_sessions.end()) {
        auto& sess = it.value();
        if (flushSession(it.key(), sess, current)) dirty = true;

        if (sess.proc && sess.proc->state() != QProcess::NotRunning) {
            sess.last = current;
            ++it;
        } else {
            it = m_sessions.erase(it);
        }
    }

    if (dirty) m_store->save();
    return dirty;
}

bool PlaytimeTracker::flushSession(const QString& key, Session& sess, float now)
{
    float elapsed = now - sess.last;
    if (elapsed <= 0) return false;
    m_store->add(key, elapsed);
    return true;
}

void PlaytimeTracker::flush()
{
    tick();
    m_sessions.clear();
    m_store->save();
}

bool PlaytimeTracker::isRunning(const Game& game) const
{
    QString key = PlaytimeStore::gameKey(game);
    auto it = m_sessions.find(key);
    if (it == m_sessions.end()) return false;
    return it->proc && it->proc->state() != QProcess::NotRunning;
}

bool PlaytimeTracker::stop(const Game& game)
{
    QString key = PlaytimeStore::gameKey(game);
    auto it = m_sessions.find(key);
    if (it == m_sessions.end()) return false;

    auto& sess = it.value();
    if (sess.proc && sess.proc->state() != QProcess::NotRunning) {
        flushSession(key, sess, nowSeconds());

        sess.proc->terminate();
        if (!sess.proc->waitForFinished(2000)) {
            sess.proc->kill();
            sess.proc->waitForFinished(1000);
        }
    }

    m_sessions.erase(it);
    m_store->save();
    return true;
}

QList<Game> PlaytimeTracker::recentlyPlayed(const QList<Game>& games, int limit) const
{
    QList<Game> ranked;
    for (const auto& g : games) {
        QString key = PlaytimeStore::gameKey(g);
        if (m_store->lastPlayed(key) > 0) {
            ranked.append(g);
        }
    }

    std::sort(ranked.begin(), ranked.end(), [this](const Game& a, const Game& b) {
        return m_store->lastPlayed(PlaytimeStore::gameKey(a))
             > m_store->lastPlayed(PlaytimeStore::gameKey(b));
    });

    while (ranked.size() > limit) ranked.removeLast();
    return ranked;
}
