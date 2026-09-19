#include "core/Game.h"
#include "core/Paths.h"

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
