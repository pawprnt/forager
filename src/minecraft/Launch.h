#pragma once

#include "minecraft/VersionJson.h"
#include "minecraft/AuthSession.h"
#include "minecraft/JavaDetect.h"

#include <QString>
#include <QStringList>
#include <memory>
#include <optional>

class QProcess;

namespace mc {

struct LaunchRequest {
    QString instanceDir;
    QString gameRoot;
    QString versionsDir;
    QString librariesRoot;
    QString assetsRoot;
    QString versionId;
};

struct LaunchPlan {
    QString javaPath;
    QStringList jvmArgs;
    QStringList gameArgs;
    QString mainClass;
    QString workingDir;
    QString nativesDir;
    QString error;
};

std::optional<LaunchPlan> buildLaunchPlan(const LaunchRequest& req, const AuthSession& session);
std::unique_ptr<QProcess> execLaunch(const LaunchPlan& plan);

std::unique_ptr<QProcess> launchInstance(const QString& instanceDir, const AuthSession& session);

} // namespace mc
