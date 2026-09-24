#pragma once

#include "minecraft/VersionJson.h"

#include <QString>
#include <QStringList>

namespace mc {

struct AssetsResult {
    bool ok = false;
    QString error;
    QString assetsDir;
    QString indexId;
};

AssetsResult ensureAssets(const VersionJson& version, const QString& assetsRoot,
                          const QString& librariesRoot);

bool ensureClientJar(const VersionJson& version, const QString& librariesRoot,
                     QString* clientJarPath, QString* error);

} // namespace mc
