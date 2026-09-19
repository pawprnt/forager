#include "artwork/Pipeline.h"
#include "artwork/Cache.h"
#include "artwork/Placeholder.h"
#include "artwork/PixmapUtils.h"
#include <QPixmap>
#include <QFile>
#include <QDir>

void art::registerPlaceholderFont() {}

QPixmap art::loadGrid(const Game& game, bool allowNetwork) {
    Q_UNUSED(allowNetwork);
    return placeholder::placeholderGrid(game.name(), 165, 248);
}

QPixmap art::loadHero(const Game& game, bool allowNetwork) {
    Q_UNUSED(allowNetwork);
    return {};
}

QPixmap art::loadIcon(const Game& game, bool allowNetwork) {
    Q_UNUSED(allowNetwork);
    return {};
}

QByteArray art::loadGridBytes(const Game& game) {
    Q_UNUSED(game);
    return {};
}

QByteArray art::loadHeroBytes(const Game& game) {
    Q_UNUSED(game);
    return {};
}
