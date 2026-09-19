#include "services/IconProvider.h"
#include "artwork/Pipeline.h"
#include <QPixmap>

QPixmap iconprovider::loadIcon(const Game& game, bool allowNetwork) {
    Q_UNUSED(game); Q_UNUSED(allowNetwork);
    return {};
}

QByteArray iconprovider::loadIconBytes(const Game& game) {
    Q_UNUSED(game);
    return {};
}
