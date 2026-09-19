#pragma once
#include <QString>
#include <QStringList>

struct ToolUpdate {
    QString name;
    QString currentVersion;
    QString latestVersion;
    QString downloadUrl;
};

namespace toolupdates {
    QList<ToolUpdate> checkToolUpdates();
    QStringList updateToolUpdates();
} // namespace toolupdates
