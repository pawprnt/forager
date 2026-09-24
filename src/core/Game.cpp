#include "core/Game.h"
#include "core/Paths.h"

#include <QSet>
#include <QDir>

static const QSet<QString> GENERIC_CONTAINERS = {
    "standalone", "series", "minecraft", "steam", "rpgmaker", "rpg",
    "games", "instances", "launcher", "single", "flash", "drm-free",
};

static const QSet<QString> ENGINE_NAMES = {"other", "rpgmaker", "unity", "unreal"};

Game::Game(QString name, Source source)
    : m_name(std::move(name))
    , m_source(source)
{
}

QString Game::identifier() const
{
    if (!m_app_id.isEmpty()) return m_app_id;
    if (!m_path.isEmpty()) return m_path;
    return m_name;
}

std::optional<std::pair<QStringList, QString>> Game::sgdbSearch() const
{
    if (!m_search_names.isEmpty())
        return std::make_pair(m_search_names, QString());
    if (m_path.isEmpty()) return std::nullopt;
    if (m_source == Source::Steam) return std::nullopt;

    QString gamesRoot = paths::gamesDir();
    if (gamesRoot.isEmpty()) return std::nullopt;

    QString canonical = QFileInfo(m_path).canonicalFilePath();
    if (canonical.isEmpty()) canonical = m_path;
    if (!canonical.startsWith(gamesRoot)) return std::nullopt;

    QString rel = canonical.mid(gamesRoot.length());
    if (rel.startsWith('/')) rel.remove(0, 1);
    if (rel.isEmpty()) return std::nullopt;

    QStringList parts = rel.split('/', Qt::SkipEmptyParts);
    if (parts.size() >= 2 && ENGINE_NAMES.contains(parts[parts.size() - 2].toLower()))
        return std::make_pair(QStringList{parts.last()}, QString());

    while (parts.size() >= 2 && GENERIC_CONTAINERS.contains(parts[parts.size() - 2].toLower()))
        parts.removeAt(parts.size() - 2);

    if (parts.size() >= 2)
        return std::make_pair(QStringList{parts[parts.size() - 2]}, parts.last());
    return std::nullopt;
}

QString Game::displayPath() const
{
    if (m_path.isEmpty()) return QStringLiteral("\u2014");

    QFileInfo info(m_path);
    QString canonical = info.canonicalFilePath();
    if (canonical.isEmpty()) return m_path;

    QString gamesRoot = paths::gamesDir();
    if (canonical.startsWith(gamesRoot)) {
        QString rel = canonical.mid(gamesRoot.length());
        if (rel.startsWith('/')) rel.remove(0, 1);
        return rel;
    }
    return m_path;
}

bool Game::operator==(const Game& other) const
{
    return m_source == other.m_source && identifier() == other.identifier();
}
