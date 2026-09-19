#include "providers/steam/SteamLibrary.h"
#include "providers/steam/SteamCredentials.h"
#include "utils/Network.h"
#include "core/Paths.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDir>
#include <QFileInfo>

std::vector<OwnedGame> SteamLibrary::fetchOwnedGames() const
{
    const auto& creds = SteamCredentials::instance();
    if (creds.webApiKey().isEmpty() || creds.steamId().isEmpty()) {
        return {};
    }

    QString url = QStringLiteral(
        "https://api.steampowered.com/IPlayerService/GetOwnedGames/v1/"
        "?key=%1&steamid=%2&include_appinfo=1&include_played_free_games=1")
        .arg(creds.webApiKey(), creds.steamId());

    QByteArray data = net::httpGet(url);
    std::vector<OwnedGame> games;
    parseOwnedResponse(data, games);
    return games;
}

std::vector<OwnedGame> SteamLibrary::fetchInstalledGames() const
{
    std::vector<OwnedGame> games;
    QString steamDir = QDir::homePath() + "/.local/share/Steam/steamapps";

    QDir dir(steamDir);
    if (!dir.exists()) return games;

    QStringList manifests = dir.entryList(QStringList() << "appmanifest_*.acf", QDir::Files);

    for (const QString& file : manifests) {
        QFile f(dir.absoluteFilePath(file));
        if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) continue;

        QString appId;
        QString name;
        QByteArray content = f.readAll();
        QStringList lines = QString::fromUtf8(content).split('\n');

        for (const QString& line : lines) {
            QString trimmed = line.trimmed();
            if (trimmed.startsWith("\"appid\""))
                appId = trimmed.section('"', 3, 3);
            else if (trimmed.startsWith("\"name\""))
                name = trimmed.section('"', 3, 3);
        }

        if (!appId.isEmpty() && !name.isEmpty()) {
            OwnedGame game;
            game.app_id = appId;
            game.name = name;
            game.provider = "steam";
            game.installed = true;
            games.push_back(std::move(game));
        }
    }

    return games;
}

bool SteamLibrary::parseOwnedResponse(const QByteArray& data, std::vector<OwnedGame>& out)
{
    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (!doc.isObject()) return false;

    QJsonObject resp = doc.object()["response"].toObject();
    QJsonArray games = resp["games"].toArray();

    for (const auto& v : games) {
        QJsonObject obj = v.toObject();
        OwnedGame game;
        game.app_id = QString::number(obj["appid"].toInt());
        game.name = obj["name"].toString();
        game.provider = "steam";
        out.push_back(std::move(game));
    }

    return true;
}
