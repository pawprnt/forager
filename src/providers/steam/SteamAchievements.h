#pragma once

#include <QString>
#include <QVector>

struct Achievement {
    QString apiName;
    QString displayName;
    QString description;
    bool achieved = false;
    qint64 unlockTime = 0;
};

struct VdfEntry {
    QString key;
    QString value;
    QVector<VdfEntry> children;
};

class SteamAchievements {
public:
    QVector<Achievement> readFromFile(const QString& steamId, const QString& appId) const;

    static VdfEntry parseVdf(const QByteArray& data);
    static QVector<Achievement> extractAchievements(const VdfEntry& root);
};
