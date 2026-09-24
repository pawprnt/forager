#include "minecraft/AuthSession.h"
#include <QCryptographicHash>
#include <QRegularExpression>

namespace mc {

QString offlineUuid(const QString& username) {
    QByteArray md5 = QCryptographicHash::hash(
        QByteArrayLiteral("OfflinePlayer:") + username.toUtf8(), QCryptographicHash::Md5);
    md5[6] = static_cast<char>((md5[6] & 0x0f) | 0x30);
    md5[8] = static_cast<char>((md5[8] & 0x3f) | 0x80);
    return QString::fromLatin1(md5.toHex());
}

bool validUsername(const QString& username) {
    static const QRegularExpression re(QStringLiteral("^[a-zA-Z0-9_]{2,16}$"));
    return re.match(username).hasMatch();
}

AuthSession makeOfflineSession(const QString& username) {
    AuthSession session;
    if (!validUsername(username)) return session;
    session.playerName = username;
    session.uuid = offlineUuid(username);
    session.accessToken = QStringLiteral("offline");
    session.userType = QStringLiteral("offline");
    session.session = QStringLiteral("-");
    session.userProperties = QStringLiteral("{}");
    session.demo = false;
    return session;
}

QMap<QString, QString> tokenMap(const AuthSession& session, const QString& profileName,
                                 const QString& versionName, const QString& versionType,
                                 const QString& gameDirectory, const QString& assetsRoot,
                                 const QString& assetsIndexName) {
    QMap<QString, QString> tokens;
    tokens[QStringLiteral("auth_session")] = session.session;
    tokens[QStringLiteral("auth_access_token")] = session.accessToken;
    tokens[QStringLiteral("auth_player_name")] = session.playerName;
    tokens[QStringLiteral("auth_uuid")] = session.uuid;
    tokens[QStringLiteral("user_properties")] = session.userProperties;
    tokens[QStringLiteral("user_type")] = session.userType;
    tokens[QStringLiteral("profile_name")] = profileName;
    tokens[QStringLiteral("version_name")] = versionName;
    tokens[QStringLiteral("version_type")] = versionType;
    tokens[QStringLiteral("game_directory")] = gameDirectory;
    tokens[QStringLiteral("game_assets")] = assetsRoot;
    tokens[QStringLiteral("assets_root")] = assetsRoot;
    tokens[QStringLiteral("assets_index_name")] = assetsIndexName;
    return tokens;
}

static QString substitutePart(const QString& part, const QMap<QString, QString>& tokens) {
    QString out;
    out.reserve(part.size());
    int i = 0;
    while (i < part.size()) {
        if (part.at(i) == QLatin1Char('$') && i + 1 < part.size() &&
            part.at(i + 1) == QLatin1Char('{')) {
            int end = part.indexOf(QLatin1Char('}'), i + 2);
            if (end != -1) {
                out += tokens.value(part.mid(i + 2, end - i - 2));
                i = end + 1;
                continue;
            }
        }
        out += part.at(i);
        ++i;
    }
    return out;
}

QStringList substituteArgs(const QString& argString, const QMap<QString, QString>& tokens) {
    QStringList result;
    const QStringList parts = argString.split(QLatin1Char(' '), Qt::SkipEmptyParts);
    for (const QString& part : parts) {
        QString sub = substitutePart(part, tokens);
        if (!sub.isEmpty()) result.append(sub);
    }
    return result;
}

QStringList substituteArgs(const QVector<QString>& args, const QMap<QString, QString>& tokens) {
    QStringList result;
    for (const QString& arg : args) {
        QString sub = substitutePart(arg, tokens);
        if (!sub.isEmpty()) result.append(sub);
    }
    return result;
}

} // namespace mc
