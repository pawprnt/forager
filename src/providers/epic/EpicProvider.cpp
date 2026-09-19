#include "providers/epic/EpicProvider.h"

#include <QProcess>
#include <QJsonDocument>
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
    QProcess proc;
    proc.setProgram(legendaryPath());
    proc.setArguments({"--version"});
    proc.start();
    proc.waitForFinished(5000);
    return proc.exitCode() == 0;
}

bool EpicProvider::isConfigured() const
{
    if (!isLegendaryInstalled()) return false;

    QProcess proc;
    proc.setProgram(legendaryPath());
    proc.setArguments({"auth", "status"});
    proc.start();
    proc.waitForFinished(5000);
    return proc.exitCode() == 0;
}

std::vector<OwnedGame> EpicProvider::listOwned(const QString& account) const
{
    if (!isConfigured()) {
        throw BackendNotConfigured("Legendary CLI not installed or not authenticated");
    }

    QProcess proc;
    proc.setProgram(legendaryPath());
    proc.setArguments({"list-games", "--json"});
    proc.start();
    proc.waitForFinished(30000);

    if (proc.exitCode() != 0) {
        throw ProviderError("legendary list-games failed: " +
                            proc.readAllStandardError().left(500).toStdString());
    }

    QByteArray output = proc.readAllStandardOutput();
    QJsonDocument doc = QJsonDocument::fromJson(output);
    std::vector<OwnedGame> games;

    if (!doc.isObject()) return games;

    QJsonArray entries = doc.object()["games"].toArray();
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
    if (!isConfigured()) {
        throw BackendNotConfigured("Legendary CLI not installed or not authenticated");
    }

    QProcess proc;
    proc.setProgram(legendaryPath());
    proc.setArguments({"install", appId, "--install-dir", destination});
    proc.setProcessChannelMode(QProcess::SeparateChannels);
    proc.start();

    while (proc.state() != QProcess::NotRunning) {
        if (cancel && cancel->load()) {
            proc.kill();
            throw ProviderError("Download cancelled");
        }
        proc.waitForReadyRead(500);

        if (onProgress) {
            DownloadProgress prog;
            prog.name = appId;
            onProgress(prog);
        }
    }

    if (proc.exitCode() != 0) {
        throw ProviderError("legendary install failed: " +
                            proc.readAllStandardError().left(500).toStdString());
    }

    if (onProgress) {
        DownloadProgress final_;
        final_.name = appId;
        final_.fraction = 1.0;
        onProgress(final_);
    }
}
