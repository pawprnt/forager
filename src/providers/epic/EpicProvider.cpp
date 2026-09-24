#include "providers/epic/EpicProvider.h"
#include "utils/Subprocess.h"
#include "utils/Json.h"

#include <QJsonObject>
#include <QJsonArray>

static AutoRegister<EpicProvider> s_reg;

EpicProvider::EpicProvider() = default;

QString EpicProvider::legendaryPath() const
{
    return QStringLiteral("legendary");
}

bool EpicProvider::isLegendaryInstalled() const
{
    return subprocess::runCheckedOptional(legendaryPath(), {"--version"}, {}, 5000).exitCode == 0;
}

bool EpicProvider::isConfigured() const
{
    if (!isLegendaryInstalled()) return false;
    return subprocess::runCheckedOptional(legendaryPath(), {"auth", "status"}, {}, 5000).exitCode == 0;
}

std::vector<OwnedGame> EpicProvider::listOwned(const QString& account) const
{
    requireConfigured("Legendary CLI not installed or not authenticated");

    subprocess::Result r = subprocess::runCheckedOptional(legendaryPath(), {"list-games", "--json"}, {}, 30000);
    if (r.exitCode != 0) {
        throw ProviderError("legendary list-games failed: " +
                            r.stderr_data.left(500).toStdString());
    }

    QJsonArray entries;
    if (auto obj = json::parseObject(r.stdout_data)) {
        entries = (*obj)["games"].toArray();
    }
    std::vector<OwnedGame> games;

    for (const auto& v : entries) {
        QJsonObject obj = v.toObject();
        OwnedGame game;
        game.app_id = obj["app_name"].toString();
        game.name = obj["app_title"].toString();
        game.provider = "epic";
        game.installed = obj["is_installed"].toBool();
        games.push_back(std::move(game));
    }

    return games;
}

void EpicProvider::download(const QString& appId, const QString& destination,
                            ProgressFn onProgress, std::atomic<bool>* cancel)
{
    requireConfigured("Legendary CLI not installed or not authenticated");

    auto r = subprocess::runStreaming(legendaryPath(),
        {"install", appId, "--install-dir", destination}, cancel, 500,
        [&](const QByteArray&) {
            if (onProgress) onProgress(DownloadProgress{appId});
        });
    if (r.cancelled) throw ProviderError("Download cancelled");
    if (r.exitCode != 0) {
        throw ProviderError("legendary install failed: " +
                            r.stderr_data.left(500).toStdString());
    }

    emitFinalProgress(appId, onProgress);
}
