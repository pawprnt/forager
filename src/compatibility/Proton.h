#pragma once
#include <QString>
#include <functional>
#include <atomic>

struct DownloadProgress {
    QString name;
    double fraction = 0.0;
    qint64 bytesReceived = 0;
    qint64 totalBytes = 0;
    int speedBps = 0;
    int etaSeconds = -1;
};

using ProgressFn = std::function<void(const DownloadProgress&)>;

namespace proton {
    bool isAvailable();
    QString launchExe(const QString& gamePath, const QString& exePath);
    QString updateProton(
        std::function<void(const QString&)> messageFn,
        ProgressFn progressFn,
        std::atomic<bool>* cancel = nullptr);
    bool ensureDepotDownloader();
    bool ensureSteamCmd();
} // namespace proton
