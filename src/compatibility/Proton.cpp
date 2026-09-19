#include "compatibility/Proton.h"
#include "core/Paths.h"
#include "utils/Subprocess.h"
#include <QDir>
#include <QFile>

bool proton::isAvailable() {
    return QFile::exists("/usr/bin/steam") || QFile::exists(QDir::homePath() + "/.steam/steam/ubuntu12_32/steam");
}

QString proton::launchExe(const QString& gamePath, const QString& exePath) {
    Q_UNUSED(gamePath); Q_UNUSED(exePath);
    return {};
}

QString proton::updateProton(std::function<void(const QString&)> messageFn,
                              ProgressFn progressFn, std::atomic<bool>* cancel) {
    Q_UNUSED(messageFn); Q_UNUSED(progressFn); Q_UNUSED(cancel);
    return {};
}

bool proton::ensureDepotDownloader() {
    QString ddDir = paths::cacheDir() + "/depotdownloader";
    return QDir(ddDir).exists();
}

bool proton::ensureSteamCmd() {
    return QFile::exists(paths::cacheDir() + "/steamcmd/steamcmd.sh");
}
