#pragma once

#include "providers/Provider.h"
#include "providers/steam/SteamCredentials.h"
#include "providers/steam/SteamLibrary.h"
#include "providers/steam/SteamDownloader.h"

#include <memory>

class SteamProvider : public Provider {
public:
    static constexpr const char* providerName() { return "steam"; }

    SteamProvider();

    QString name() const override { return QStringLiteral("steam"); }
    bool isConfigured() const override;
    std::vector<OwnedGame> listOwned(const QString& account = {}) const override;
    void download(const QString& appId, const QString& destination,
                  ProgressFn onProgress = nullptr,
                  std::atomic<bool>* cancel = nullptr) override;

private:
    std::unique_ptr<SteamLibrary> m_library;
    std::unique_ptr<SteamDownloader> m_downloader;
};
