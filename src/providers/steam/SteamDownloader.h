#pragma once

#include "providers/Provider.h"
#include <atomic>

class SteamDownloader {
public:
    void download(const QString& appId, const QString& destination,
                  ProgressFn onProgress = nullptr,
                  std::atomic<bool>* cancel = nullptr);

private:
    QString depotDownloaderPath() const;
    bool isInstalled() const;
    void requireConfigured() const;
};
