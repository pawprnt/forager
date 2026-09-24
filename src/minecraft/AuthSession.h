#pragma once

#include <QString>
#include <QStringList>
#include <QMap>

namespace mc {

struct AuthSession {
    QString playerName;
    QString uuid;
    QString accessToken = QStringLiteral("offline");
    QString userType = QStringLiteral("offline");
    QString session = QStringLiteral("-");
    QString userProperties = QStringLiteral("{}");
    bool demo = false;
};

AuthSession makeOfflineSession(const QString& username);
QString offlineUuid(const QString& username);
bool validUsername(const QString& username);

QMap<QString, QString> tokenMap(const AuthSession& session, const QString& profileName,
                                 const QString& versionName, const QString& versionType,
                                 const QString& gameDirectory, const QString& assetsRoot,
                                 const QString& assetsIndexName);

QStringList substituteArgs(const QString& argString, const QMap<QString, QString>& tokens);
QStringList substituteArgs(const QVector<QString>& args, const QMap<QString, QString>& tokens);

} // namespace mc
