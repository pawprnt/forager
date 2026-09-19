#include "providers/steam/SteamDownloader.h"
#include "providers/steam/SteamCredentials.h"
#include "utils/Network.h"
#include "core/Paths.h"

#include <QProcess>
#include <QDir>
#include <QRegularExpression>
#include <QJsonDocument>
#include <QJsonObject>

QString SteamDownloader::depotDownloaderPath() const
{
    return paths::cacheDir() + "/depotdownloader/DepotDownloader";
}

bool SteamDownloader::isInstalled() const
{
    return QFile::exists(depotDownloaderPath());
}

void SteamDownloader::download(const QString& appId, const QString& destination,
                               ProgressFn onProgress, std::atomic<bool>* cancel)
{
    if (!isInstalled()) {
        throw ProviderError("DepotDownloader not found at: " +
                            depotDownloaderPath().toStdString());
    }

    const auto& creds = SteamCredentials::instance();
    if (!creds.hasCredentials()) {
        throw BackendNotConfigured("Steam credentials required for download");
    }

    QDir().mkpath(destination);

    QStringList args;
    args << "-app" << appId
         << "-dir" << destination
         << "-username" << creds.username();

    if (!creds.loginSecure().isEmpty()) {
        args << "-remember-password";
    }

    QProcess proc;
    proc.setProgram(depotDownloaderPath());
    proc.setArguments(args);
    proc.setProcessChannelMode(QProcess::SeparateChannels);
    proc.start();

    QRegularExpression progressRe(R"((\d+\.?\d*)%\s*--\s*(\d+\.?\d*)\s*MB\s*/\s*(\d+\.?\d*)\s*MB)");
    QRegularExpression speedRe(R"((\d+\.?\d*)\s*(KB|MB|GB)/s)");

    while (proc.state() != QProcess::NotRunning) {
        if (cancel && cancel->load()) {
            proc.kill();
            throw ProviderError("Download cancelled");
        }

        proc.waitForReadyRead(200);
        QByteArray line = proc.readAllStandardOutput();

        for (const QByteArray& l : line.split('\n')) {
            QString str = QString::fromUtf8(l).trimmed();
            if (str.isEmpty()) continue;

            DownloadProgress prog;
            prog.name = appId;

            auto pm = progressRe.match(str);
            if (pm.hasMatch()) {
                prog.fraction = pm.captured(1).toDouble() / 100.0;
                prog.bytes_received = static_cast<qint64>(pm.captured(2).toDouble() * 1024 * 1024);
                prog.total_bytes = static_cast<qint64>(pm.captured(3).toDouble() * 1024 * 1024);
            }

            auto sm = speedRe.match(str);
            if (sm.hasMatch()) {
                double val = sm.captured(1).toDouble();
                QString unit = sm.captured(2);
                if (unit == "KB") val *= 1024;
                else if (unit == "MB") val *= 1024 * 1024;
                else if (unit == "GB") val *= 1024 * 1024 * 1024;
                prog.speed_bps = static_cast<int>(val);
            }

            if (onProgress) onProgress(prog);
        }
    }

    if (proc.exitCode() != 0) {
        QByteArray err = proc.readAllStandardError();
        throw ProviderError("DepotDownloader failed (exit " +
                            std::to_string(proc.exitCode()) + "): " +
                            err.left(500).toStdString());
    }

    if (onProgress) {
        DownloadProgress final_;
        final_.name = appId;
        final_.fraction = 1.0;
        onProgress(final_);
    }
}
