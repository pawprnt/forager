#include "artwork/PipelineInternal.h"
#include "core/Paths.h"
#include "core/Game.h"

// --- Steam appcache ---

QPixmap art::detail::fromSteamAppcache(const Game& game, const QString& filename) {
    if (!isSteamLocal(game)) return {};
    QString dir = paths::steamAppcacheDir();
    if (dir.isEmpty()) return {};
    return loadPixmapFile(dir + "/" + game.appId() + "/" + filename);
}

bool art::detail::isSteamLocal(const Game& game) {
    return game.source() == Source::Steam && !game.appId().isEmpty();
}
