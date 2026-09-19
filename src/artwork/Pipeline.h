#pragma once
#include "core/Game.h"
#include <QPixmap>
#include <QByteArray>

namespace art {
    QPixmap loadGrid(const Game& game, bool allowNetwork = true);
    QPixmap loadHero(const Game& game, bool allowNetwork = true);
    QPixmap loadIcon(const Game& game, bool allowNetwork = true);
    QByteArray loadGridBytes(const Game& game);
    QByteArray loadHeroBytes(const Game& game);
    void registerPlaceholderFont();
} // namespace art
