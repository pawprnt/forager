#include "artwork/PipelineInternal.h"
#include "utils/Network.h"

static const char* STEAM_CDN = "https://shared.akamai.steamstatic.com/store_item_assets/steam/apps";

// --- Steam CDN ---

QByteArray art::detail::fetchSteamCDN(const QString& appId, const QString& filename) {
    if (appId.isEmpty()) return {};
    QString url = QStringLiteral("%1/%2/%3").arg(STEAM_CDN, appId, filename);
    return net::httpGet(url, 10000);
}
