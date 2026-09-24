#include "providers/gog/GogProvider.h"
#include "utils/Network.h"
#include "utils/Json.h"
#include "utils/Filesystem.h"
#include "core/Paths.h"

#include <QJsonObject>
#include <QJsonArray>

static AutoRegister<GogProvider> s_reg;

GogProvider::GogProvider() = default;

void GogProvider::setBearerToken(const QString& token)
{
    m_bearerToken = token;
}

bool GogProvider::isConfigured() const
{
    return !m_bearerToken.isEmpty();
}

QJsonObject GogProvider::getFilteredProducts() const
{
    QString url = QStringLiteral(
        "https://embed.gog.com/games/ajax/filtered?mediaType=game");

    QByteArray data = net::httpGet(url, 15000, net::bearerHeaders(m_bearerToken));
    auto obj = json::parseObject(data);
    if (obj) return *obj;
    return {};
}

std::vector<OwnedGame> GogProvider::listOwned(const QString& account) const
{
    requireConfigured("GOG bearer token not configured");

    QJsonObject products = getFilteredProducts();
    std::vector<OwnedGame> games;

    QJsonArray items = products["products"].toArray();
    for (const auto& v : items) {
        QJsonObject obj = v.toObject();
        OwnedGame game;
        game.app_id = QString::number(obj["id"].toInt());
        game.name = obj["title"].toString();
        game.provider = "gog";
        game.installed = false;
        games.push_back(std::move(game));
    }

    return games;
}

void GogProvider::download(const QString& appId, const QString& destination,
                           ProgressFn onProgress, std::atomic<bool>* cancel)
{
    requireConfigured("GOG bearer token not configured");

    QString url = QStringLiteral(
        "https://api.gog.com/products/%1?expand=downloads")
        .arg(appId);

    QByteArray data = net::httpGet(url, 15000, net::bearerHeaders(m_bearerToken));
    auto obj = json::parseObject(data);
    if (!obj) {
        throw ProviderError("Failed to fetch GOG product details");
    }

    QJsonArray downloads = (*obj)["downloads"].toArray();
    for (const auto& v : downloads) {
        QJsonObject dl = v.toObject();
        QJsonArray files = dl["files"].toArray();
        for (const auto& f : files) {
            QJsonObject file = f.toObject();
            QString installerUrl = file["url"].toString();
            if (!installerUrl.isEmpty()) {
                QString filename = installerUrl.section('/', -1);
                QString outPath = destination + "/" + filename;
                QByteArray fileData = net::httpGet(installerUrl, 300000);
                fs::writeBytes(outPath, fileData);
                emitFinalProgress(appId, onProgress);
                return;
            }
        }
    }

    throw ProviderError("No downloadable files found for GOG app: " +
                        appId.toStdString());
}
