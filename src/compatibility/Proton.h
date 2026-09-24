#pragma once
#include "providers/Provider.h"
#include <atomic>

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
