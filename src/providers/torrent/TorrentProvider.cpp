#include "providers/torrent/TorrentProvider.h"

static AutoRegister<TorrentProvider> s_reg;

TorrentProvider::TorrentProvider() = default;

bool TorrentProvider::isLibtorrentAvailable() const
{
    return false;
}

bool TorrentProvider::isConfigured() const
{
    return isLibtorrentAvailable();
}

std::vector<OwnedGame> TorrentProvider::listOwned(const QString& account) const
{
    return {};
}

void TorrentProvider::download(const QString& appId, const QString& destination,
                               ProgressFn onProgress, std::atomic<bool>* cancel)
{
    if (!isConfigured()) {
        throw BackendNotConfigured("libtorrent not available");
    }
}
