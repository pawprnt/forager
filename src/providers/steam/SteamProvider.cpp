#include "providers/steam/SteamProvider.h"
#include "providers/steam/SteamCredentials.h"

#include <algorithm>

static AutoRegister<SteamProvider> s_reg;

SteamProvider::SteamProvider()
    : m_library(std::make_unique<SteamLibrary>())
    , m_downloader(std::make_unique<SteamDownloader>())
{
}

bool SteamProvider::isConfigured() const
{
    return SteamCredentials::instance().hasCredentials();
}

std::vector<OwnedGame> SteamProvider::listOwned(const QString& account) const
{
    if (!isConfigured()) {
        throw BackendNotConfigured("Steam credentials not configured");
    }

    auto owned = m_library->fetchOwnedGames();

    auto installed = m_library->fetchInstalledGames();
    for (auto& game : installed) {
        auto it = std::find_if(owned.begin(), owned.end(),
            [&](const OwnedGame& g) { return g.app_id == game.app_id; });
        if (it != owned.end()) {
            it->installed = true;
        } else {
            game.installed = true;
            owned.push_back(std::move(game));
        }
    }

    return owned;
}

void SteamProvider::download(const QString& appId, const QString& destination,
                             ProgressFn onProgress, std::atomic<bool>* cancel)
{
    if (!isConfigured()) {
        throw BackendNotConfigured("Steam credentials not configured");
    }

    m_downloader->download(appId, destination, onProgress, cancel);
}
