#pragma once

#include "core/Game.h"
#include <QProcess>
#include <memory>

namespace launcher {
    std::unique_ptr<QProcess> launch(const Game& game);
} // namespace launcher
