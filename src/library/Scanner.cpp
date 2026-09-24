#include "library/Scanner.h"
#include "library/Metadata.h"
#include "core/Paths.h"
#include "utils/Acf.h"

#include <QDir>
#include <QFileInfo>
#include <QSet>
#include <algorithm>

static const QSet<QString> STEAM_TOOL_APP_IDS = {
    "1493710", "250820", "1070560", "1391110", "1628350"
};

static const QSet<QString> GENERIC_CONTAINERS = {
    "standalone", "series", "minecraft", "steam", "rpgmaker", "rpg",
    "games", "instances", "launcher", "single", "flash", "drm-free",
};

static const QSet<QString> ENGINE_NAMES = {"other", "rpgmaker", "unity", "unreal"};

template <typename Fn>
static void forEachSubdir(const QString& path, Fn fn)
{
    QDir dir(path);
    dir.setFilter(QDir::Dirs | QDir::NoDotAndDotDot);
    dir.setSorting(QDir::Name);
    for (const auto& entry : dir.entryInfoList()) {
        if (entry.fileName().startsWith('.')) continue;
        fn(entry);
    }
}

// --- Steam scanner ---

static std::vector<Game> scanSteam()
{
    std::vector<Game> games;
    for (const QString& file : acf::listManifests(paths::gamesDir() + "/steam/steamapps")) {
        auto [appId, name] = acf::parseFile(file);
        if (appId.isEmpty() || name.isEmpty()) continue;
        if (STEAM_TOOL_APP_IDS.contains(appId)) continue;

        Game g(name, Source::Steam);
        g.setAppId(appId);
        g.setPath(QFileInfo(file).absolutePath() + "/common/" + name);
        g.setSortKey(name.toLower());
        games.push_back(std::move(g));
    }
    return games;
}

// --- Minecraft scanner ---

static std::vector<Game> scanMinecraft()
{
    std::vector<Game> games;
    QString root = paths::gamesDir() + "/minecraft";
    if (!QDir(root).exists()) return games;

    forEachSubdir(root, [&](const QFileInfo& entry) {
        QString name = entry.fileName();
        Game g(name, Source::Minecraft);
        g.setPath(entry.absoluteFilePath());
        g.setSortKey(name.toLower());
        games.push_back(std::move(g));
    });
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
    forEachSubdir(dirPath, [&](const QFileInfo& entry) {
        out.push_back(makeLooseGame(entry));
    });
}

static void scanLooseRoot(const QString& rootPath, std::vector<Game>& out)
{
    forEachSubdir(rootPath, [&](const QFileInfo& entry) {
        if (isGameDir(entry.absoluteFilePath())) {
            out.push_back(makeLooseGame(entry));
        } else {
            scanLooseFlat(entry.absoluteFilePath(), out);
        }
    });
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

    forEachSubdir(path, [&](const QFileInfo& entry) {
        QStringList newParts = parts;
        if (!ENGINE_NAMES.contains(entry.fileName().toLower())) {
            newParts.append(entry.fileName());
        }
        collectSeries(entry.absoluteFilePath(), newParts, out);
    });
}

static void scanSeriesDir(const QString& seriesRoot, std::vector<Game>& out)
{
    forEachSubdir(seriesRoot, [&](const QFileInfo& entry) {
        if (isGameDir(entry.absoluteFilePath())) {
            out.push_back(makeLooseGame(entry));
            return;
        }
        QStringList parts;
        if (!ENGINE_NAMES.contains(entry.fileName().toLower())) {
            parts.append(entry.fileName());
        }
        collectSeries(entry.absoluteFilePath(), parts, out);
    });
}

static std::vector<Game> scanStandalone()
{
    std::vector<Game> games;
    for (const auto& container : {"standalone", "drm-free"}) {
        QString base = paths::gamesDir() + "/" + container;
        if (!QDir(base).exists()) continue;

        forEachSubdir(base, [&](const QFileInfo& entry) {
            if (entry.fileName() == "series") {
                scanSeriesDir(entry.absoluteFilePath(), games);
            } else {
                scanLooseRoot(entry.absoluteFilePath(), games);
            }
        });
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
        return metadata::sortKey(a) < metadata::sortKey(b);
    });

    return all;
}
