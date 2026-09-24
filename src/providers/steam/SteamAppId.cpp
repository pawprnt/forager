#include "providers/steam/SteamAppId.h"
#include "utils/Network.h"
#include "utils/Json.h"
#include "core/Paths.h"

#include <QJsonObject>
#include <QJsonArray>
#include <QMutex>
#include <QMutexLocker>
#include <QRegularExpression>
#include <QUrl>

static QMutex s_cacheMutex;

static QString leafName(const QString& name)
{
    int slash = name.lastIndexOf('/');
    return slash >= 0 ? name.mid(slash + 1) : name;
}

static QString loadCacheLocked(const QString& key)
{
    auto cache = json::readObject(paths::artCacheDir() + "/steam_app_ids.json");
    if (!cache) return {};
    return (*cache)[key].toString();
}

static void storeCacheLocked(const QString& key, const QString& appId)
{
    QString path = paths::artCacheDir() + "/steam_app_ids.json";
    QJsonObject cache = json::readObject(path).value_or(QJsonObject{});
    cache[key] = appId;
    json::writeObject(path, cache, false);
}

QStringList SteamAppId::searchTerms(const Game& game)
{
    if (!game.searchNames().isEmpty())
        return game.searchNames();

    QStringList terms;
    if (auto plan = game.sgdbSearch()) {
        const auto& [queries, matchTerm] = *plan;
        if (!matchTerm.isEmpty()) {
            for (const QString& q : queries)
                terms.append(q + " " + matchTerm);
            terms.append(matchTerm);
        } else {
            terms = queries;
        }
    }

    QString leaf = leafName(game.name()).trimmed();
    if (!leaf.isEmpty() && (terms.isEmpty() || leaf != terms.last()))
        terms.append(leaf);
    return terms;
}

std::optional<QString> SteamAppId::resolve(const Game& game)
{
    if (!game.appId().isEmpty()) return game.appId();

    QStringList terms = searchTerms(game);
    if (terms.isEmpty()) return std::nullopt;

    QMutexLocker lock(&s_cacheMutex);
    for (const QString& term : terms) {
        QString key = term.toLower();
        QString cached = loadCacheLocked(key);
        if (cached.isEmpty()) {
            lock.unlock();
            auto result = searchStore(key);
            cached = result.value_or(QStringLiteral("-"));
            lock.relock();
            storeCacheLocked(key, cached);
        }
        if (!cached.isEmpty() && cached != QStringLiteral("-"))
            return cached;
    }
    return std::nullopt;
}

std::optional<QString> SteamAppId::resolveByName(const QString& name)
{
    if (name.isEmpty()) return std::nullopt;

    QString key = name.toLower();
    {
        QMutexLocker lock(&s_cacheMutex);
        QString cached = loadCacheLocked(key);
        if (!cached.isEmpty()) {
            if (cached == "-") return std::nullopt;
            return cached;
        }
    }

    auto result = searchStore(key);
    QMutexLocker lock(&s_cacheMutex);
    storeCacheLocked(key, result.value_or("-"));
    return result;
}

std::optional<QString> SteamAppId::searchStore(const QString& term)
{
    QString url = QStringLiteral(
        "https://store.steampowered.com/api/storesearch/"
        "?term=%1&l=english&cc=US")
        .arg(QString::fromUtf8(QUrl::toPercentEncoding(term)));

    QByteArray data = net::httpGet(url);
    auto obj = json::parseObject(data);
    if (!obj) return std::nullopt;

    QJsonArray items = (*obj)["items"].toArray();
    if (items.isEmpty()) return std::nullopt;

    for (const auto& v : items) {
        QJsonObject o = v.toObject();
        if (o["type"].toString() != QLatin1String("app")) continue;
        QString storeName = o["name"].toString();
        if (nameMatches(term, storeName)) {
            return QString::number(o["id"].toInt());
        }
    }

    return std::nullopt;
}

bool SteamAppId::nameMatches(const QString& query, const QString& storeName)
{
    static const QRegularExpression nonAlnum(QStringLiteral("[^a-z0-9]+"));
    QString n = QString(storeName).toLower().replace(nonAlnum, " ").trimmed();
    QString t = QString(query).toLower().replace(nonAlnum, " ").trimmed();

    if (n == t) return true;
    if (t.length() >= 8 && n.startsWith(t) && (n.length() == t.length() || n.mid(t.length()) == QLatin1String(" ")))
        return true;
    return false;
}
