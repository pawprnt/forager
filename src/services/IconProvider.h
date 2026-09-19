#pragma once
#include "core/Game.h"
#include <QPixmap>

namespace iconprovider {
    QPixmap loadIcon(const Game& game, bool allowNetwork = true);
    QByteArray loadIconBytes(const Game& game);
} // namespace iconprovider
