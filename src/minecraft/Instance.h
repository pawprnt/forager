#pragma once

#include "minecraft/VersionJson.h"

#include <QString>
#include <QStringList>

namespace mc {

std::optional<QString> findVersionJson(const QString& versionsDir, const QString& versionId);
std::optional<QString> detectVersionId(const QString& instanceDir);
QString resolveGameRoot(const QString& instanceDir);
QString resolveNativesDir(const QString& instanceDir);

} // namespace mc
