#include "providers/steam/SteamAppId.h"
#include "utils/Network.h"
#include "core/Paths.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QDir>
#include <QMutex>
#include <QMutexLocker>

static QMutex s_cacheMutex;

std::optional<QString> SteamAppId::resolve(const QString& name)
{
    if (name.isEmpty()) return std::nullopt;

    QString key = name.toLower();
    QString cached = loadCache(key);
    if (!cached.isEmpty()) {
        if (cached == "-") return std::nullopt;
        return cached;
    }

    auto result = searchStore(key);
    storeCache(key, result.value_or("-"));
    return result;
}

std::optional<QString> SteamAppId::searchStore(const QString& term)
{
    QString url = QStringLiteral(
        "https://store.steampowered.com/api/storesearch/"
        "?term=%1&l=english&cc=US")
        .arg(term);

    QByteArray data = net::httpGet(url);
    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (!doc.isObject()) return std::nullopt;

    QJsonArray items = doc.object()["items"].toArray();
    if (items.isEmpty()) return std::nullopt;

    for (const auto& v : items) {
        QJsonObject obj = v.toObject();
        QString storeName = obj["name"].toString();
        if (nameMatches(term, storeName)) {
            return QString::number(obj["id"].toInt());
        }
    }

    return std::nullopt;
}

bool SteamAppId::nameMatches(const QString& query, const QString& storeName)
{
    QString q = query.toLower().trimmed();
    QString s = storeName.toLower().trimmed();

    if (q == s) return true;

    if (q.length() >= 8 && s.startsWith(q)) return true;

    return false;
}

QString SteamAppId::cachePath()
{
    return paths::artCacheDir() + "/steam_app_ids.json";
}

QString SteamAppId::loadCache(const QString& key)
{
    QMutexLocker lock(&s_cacheMutex);
    QFile file(cachePath());
    if (!file.open(QIODevice::ReadOnly)) return {};

    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    if (!doc.isObject()) return {};

    return doc.object()[key].toString();
}

void SteamAppId::storeCache(const QString& key, const QString& appId)
{
    QMutexLocker lock(&s_cacheMutex);

    QString path = cachePath();
    QDir().mkpath(QFileInfo(path).absolutePath());

    QJsonObject cache;
    QFile readFile(path);
    if (readFile.open(QIODevice::ReadOnly)) {
        QJsonDocument doc = QJsonDocument::fromJson(readFile.readAll());
        if (doc.isObject()) cache = doc.object();
    }
    readFile.close();

    cache[key] = appId;

    QFile writeFile(path);
    if (writeFile.open(QIODevice::WriteOnly)) {
        writeFile.write(QJsonDocument(cache).toJson(QJsonDocument::Compact));
    }
}
