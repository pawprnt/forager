#include "services/IconProvider.h"
#include "artwork/Pipeline.h"
#include <QPixmap>

QPixmap iconprovider::loadIcon(const Game& game, bool allowNetwork) {
    return art::loadIcon(game, allowNetwork);
}

QByteArray iconprovider::loadIconBytes(const Game& game) {
    Q_UNUSED(game);
    return {};
}
