#include "providers/steam/SteamLibrary.h"
#include "providers/steam/SteamCredentials.h"
#include "utils/Network.h"
#include "utils/Json.h"
#include "utils/Acf.h"
#include "core/Paths.h"

#include <QJsonObject>
#include <QJsonArray>
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
    for (const QString& file : acf::listManifests(paths::steamRoot() + "/steamapps")) {
        auto [appId, name] = acf::parseFile(file);
        if (appId.isEmpty() || name.isEmpty()) continue;

        OwnedGame game;
        game.app_id = appId;
        game.name = name;
        game.provider = "steam";
        game.installed = true;
        games.push_back(std::move(game));
    }

    return games;
}

bool SteamLibrary::parseOwnedResponse(const QByteArray& data, std::vector<OwnedGame>& out)
{
    auto obj = json::parseObject(data);
    if (!obj) return false;

    QJsonArray games = (*obj)["response"].toObject()["games"].toArray();

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
