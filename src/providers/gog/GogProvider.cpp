#include "providers/gog/GogProvider.h"
#include "utils/Network.h"
#include "core/Paths.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QDir>

static AutoRegister<GogProvider> s_reg;

GogProvider::GogProvider() = default;

void GogProvider::setBearerToken(const QString& token)
{
    m_bearerToken = token;
}

net::Headers GogProvider::authHeaders() const
{
    net::Headers headers;
    if (!m_bearerToken.isEmpty()) {
        headers["Authorization"] = "Bearer " + m_bearerToken;
    }
    return headers;
}

bool GogProvider::isConfigured() const
{
    return !m_bearerToken.isEmpty();
}

QJsonObject GogProvider::getFilteredProducts() const
{
    QString url = QStringLiteral(
        "https://embed.gog.com/games/ajax/filtered?mediaType=game");

    QByteArray data = net::httpGet(url, 15000, authHeaders());
    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (doc.isObject()) return doc.object();
    return {};
}

std::vector<OwnedGame> GogProvider::listOwned(const QString& account) const
{
    if (!isConfigured()) {
        throw BackendNotConfigured("GOG bearer token not configured");
    }

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
    if (!isConfigured()) {
        throw BackendNotConfigured("GOG bearer token not configured");
    }

    QString url = QStringLiteral(
        "https://api.gog.com/products/%1?expand=downloads")
        .arg(appId);

    QByteArray data = net::httpGet(url, 15000, authHeaders());
    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (!doc.isObject()) {
        throw ProviderError("Failed to fetch GOG product details");
    }

    QJsonArray downloads = doc.object()["downloads"].toArray();
    for (const auto& v : downloads) {
        QJsonObject dl = v.toObject();
        QJsonArray files = dl["files"].toArray();
        for (const auto& f : files) {
            QJsonObject file = f.toObject();
            QString installerUrl = file["url"].toString();
            if (!installerUrl.isEmpty()) {
                QDir().mkpath(destination);
                QByteArray fileData = net::httpGet(installerUrl, 300000);
                QString filename = installerUrl.section('/', -1);
                QFile out(destination + "/" + filename);
                if (out.open(QIODevice::WriteOnly)) {
                    out.write(fileData);
                }
                if (onProgress) {
                    DownloadProgress prog;
                    prog.name = appId;
                    prog.fraction = 1.0;
                    onProgress(prog);
                }
                return;
            }
        }
    }

    throw ProviderError("No downloadable files found for GOG app: " +
                        appId.toStdString());
}
