#include "library/Scanner.h"
#include "core/Paths.h"

#include <QDir>
#include <QFileInfo>
#include <QSet>
#include <QRegularExpression>
#include <algorithm>

static const QSet<QString> STEAM_TOOL_APP_IDS = {
    "1493710", "250820", "1070560", "1391110", "1628350"
};

static const QSet<QString> GENERIC_CONTAINERS = {
    "standalone", "series", "minecraft", "steam", "rpgmaker", "rpg",
    "games", "instances", "launcher", "single", "flash", "drm-free",
};

static const QSet<QString> ENGINE_NAMES = {"other", "rpgmaker", "unity", "unreal"};

// --- ACF parser ---

static QString acfValue(const QString& text, const QString& key)
{
    QRegularExpression re(QStringLiteral("\"%1\"\\s+\"(.+?)\"").arg(key));
    auto m = re.match(text);
    return m.hasMatch() ? m.captured(1) : QString();
}

static std::pair<QString, QString> parseAcf(const QString& filePath)
{
    QFile f(filePath);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text))
        return {};

    QString text = QString::fromUtf8(f.readAll());
    QString appId = acfValue(text, "appid");
    QString name = acfValue(text, "name");
    name.remove(QChar::Null);
    return {appId, name};
}

// --- Steam scanner ---

static std::vector<Game> scanSteam()
{
    std::vector<Game> games;
    QString appsDir = paths::gamesDir() + "/steam/steamapps";
    QDir dir(appsDir);
    if (!dir.exists()) return games;

    QStringList acfs = dir.entryList({"appmanifest_*.acf"}, QDir::Files, QDir::Name);
    for (const auto& acf : acfs) {
        auto [appId, name] = parseAcf(dir.absoluteFilePath(acf));
        if (appId.isEmpty() || name.isEmpty()) continue;
        if (STEAM_TOOL_APP_IDS.contains(appId)) continue;

        Game g(name, Source::Steam);
        g.setAppId(appId);
        g.setPath(dir.absoluteFilePath("common/" + name));
        g.setSortKey(name.toLower());
        games.push_back(std::move(g));
    }
    return games;
}

// --- Minecraft scanner ---

static std::vector<Game> scanMinecraft()
{
    std::vector<Game> games;
    QDir mcDir(paths::gamesDir() + "/minecraft");
    if (!mcDir.exists()) return games;

    mcDir.setFilter(QDir::Dirs | QDir::NoDotAndDotDot);
    mcDir.setSorting(QDir::Name);
    for (const auto& entry : mcDir.entryInfoList()) {
        QString name = entry.fileName();
        if (name.startsWith('.') || name == ".LAUNCHER_TEMP") continue;

        Game g(name, Source::Minecraft);
        g.setPath(entry.absoluteFilePath());
        g.setSortKey(name.toLower());
        games.push_back(std::move(g));
    }
    return games;
}

// --- Standalone scanner ---

static bool isGameDir(const QString& path)
{
    QDir dir(path);
    if (QFileInfo::exists(path + "/Game.ini")) return true;

    for (const auto& pattern : {"*.exe", "*.x86_64", "*.sh", "*.py", "icon.png"}) {
        if (!dir.entryList({pattern}, QDir::Files).isEmpty()) return true;
    }
    return false;
}

static Game makeLooseGame(const QFileInfo& entry)
{
    QString name = entry.fileName();
    Game g(name, Source::Standalone);
    g.setPath(entry.absoluteFilePath());
    g.setSortKey(name.toLower());

    if (name == "bdcc") {
        g.setSearchNames({"Broken Dreams Correctional Center"});
    }
    return g;
}

static void collectSeries(const QString& path, QStringList parts, std::vector<Game>& out);

static void scanLooseFlat(const QString& dirPath, std::vector<Game>& out)
{
    QDir dir(dirPath);
    dir.setFilter(QDir::Dirs | QDir::NoDotAndDotDot);
    dir.setSorting(QDir::Name);
    for (const auto& entry : dir.entryInfoList()) {
        if (entry.fileName().startsWith('.')) continue;
        out.push_back(makeLooseGame(entry));
    }
}

static void scanLooseRoot(const QString& rootPath, std::vector<Game>& out)
{
    QDir root(rootPath);
    root.setFilter(QDir::Dirs | QDir::NoDotAndDotDot);
    root.setSorting(QDir::Name);
    for (const auto& entry : root.entryInfoList()) {
        if (entry.fileName().startsWith('.')) continue;
        if (isGameDir(entry.absoluteFilePath())) {
            out.push_back(makeLooseGame(entry));
        } else {
            scanLooseFlat(entry.absoluteFilePath(), out);
        }
    }
}

static void collectSeries(const QString& path, QStringList parts, std::vector<Game>& out)
{
    if (isGameDir(path)) {
        QString rel = parts.join('/');
        Game g(rel, Source::Standalone);
        g.setPath(path);
        g.setSortKey(rel);
        out.push_back(std::move(g));
        return;
    }

    QDir dir(path);
    dir.setFilter(QDir::Dirs | QDir::NoDotAndDotDot);
    dir.setSorting(QDir::Name);
    for (const auto& entry : dir.entryInfoList()) {
        if (entry.fileName().startsWith('.')) continue;
        QStringList newParts = parts;
        if (!ENGINE_NAMES.contains(entry.fileName().toLower())) {
            newParts.append(entry.fileName());
        }
        collectSeries(entry.absoluteFilePath(), newParts, out);
    }
}

static void scanSeriesDir(const QString& seriesRoot, std::vector<Game>& out)
{
    QDir dir(seriesRoot);
    dir.setFilter(QDir::Dirs | QDir::NoDotAndDotDot);
    dir.setSorting(QDir::Name);
    for (const auto& entry : dir.entryInfoList()) {
        if (entry.fileName().startsWith('.')) continue;
        if (isGameDir(entry.absoluteFilePath())) {
            out.push_back(makeLooseGame(entry));
            continue;
        }
        QStringList parts;
        if (!ENGINE_NAMES.contains(entry.fileName().toLower())) {
            parts.append(entry.fileName());
        }
        collectSeries(entry.absoluteFilePath(), parts, out);
    }
}

static std::vector<Game> scanStandalone()
{
    std::vector<Game> games;
    for (const auto& container : {"standalone", "drm-free"}) {
        QString base = paths::gamesDir() + "/" + container;
        if (!QDir(base).exists()) continue;

        QDir baseDir(base);
        baseDir.setFilter(QDir::Dirs | QDir::NoDotAndDotDot);
        baseDir.setSorting(QDir::Name);
        for (const auto& entry : baseDir.entryInfoList()) {
            if (entry.fileName().startsWith('.')) continue;
            if (entry.fileName() == "series") {
                scanSeriesDir(entry.absoluteFilePath(), games);
            } else {
                scanLooseRoot(entry.absoluteFilePath(), games);
            }
        }
    }
    return games;
}

// --- Main entry ---

std::vector<Game> scanner::scanAll()
{
    QSet<QString> seen;
    std::vector<Game> all;

    auto merge = [&](std::vector<Game>&& games) {
        for (auto& g : games) {
            QString id = g.identifier();
            if (seen.contains(id)) continue;
            seen.insert(id);
            all.push_back(std::move(g));
        }
    };

    merge(scanSteam());
    merge(scanMinecraft());
    merge(scanStandalone());
    // TODO: merge scanOwnedSteam() (requires Steam provider)

    // Filter out proton, sort
    all.erase(
        std::remove_if(all.begin(), all.end(), [](const Game& g) {
            return g.name().toLower().contains("proton");
        }),
        all.end()
    );

    std::sort(all.begin(), all.end(), [](const Game& a, const Game& b) {
        QString ka = a.sortKey().isEmpty() ? a.name().toLower() : a.sortKey();
        QString kb = b.sortKey().isEmpty() ? b.name().toLower() : b.sortKey();
        return ka < kb;
    });

    return all;
}
