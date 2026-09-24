#pragma once

#include "minecraft/VersionJson.h"

#include <QString>
#include <QStringList>

namespace mc {

struct ClasspathResult {
    QStringList jars;
    QStringList nativeJars;
    QString error;
};

ClasspathResult buildClasspath(const VersionJson& version, const QString& librariesRoot,
                               const QString& osName, const QString& osArch);

bool extractNatives(const QStringList& nativeJars, const QString& destDir,
                    const QJsonObject& extractExclude);

} // namespace mc
