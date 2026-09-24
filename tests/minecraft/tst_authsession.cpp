#include "minecraft/AuthSession.h"

#include <QtTest>

class TestAuthSession : public QObject {
    Q_OBJECT

private slots:
    void validUsernameAccepts();
    void validUsernameRejects();
    void offlineUuidNotchVector();
    void offlineUuidPlayerLength();
    void makeOfflineSessionValid();
    void makeOfflineSessionInvalid();
    void tokenMapKeys();
    void substituteArgsKnownTokens();
    void substituteArgsUnknownPlaceholder();
    void substituteArgsVector();
};

void TestAuthSession::validUsernameAccepts()
{
    QVERIFY(mc::validUsername(QStringLiteral("Player")));
    QVERIFY(mc::validUsername(QStringLiteral("ab")));
    QVERIFY(mc::validUsername(QStringLiteral("Valid_Name_123")));
}

void TestAuthSession::validUsernameRejects()
{
    QVERIFY(!mc::validUsername(QStringLiteral("a")));
    QVERIFY(!mc::validUsername(QStringLiteral("17char_name_here_xx")));
    QVERIFY(!mc::validUsername(QStringLiteral("has space")));
}

void TestAuthSession::offlineUuidNotchVector()
{
    QString uuid = mc::offlineUuid(QStringLiteral("Notch"));
    QCOMPARE(uuid, QStringLiteral("b50ad385829d3141a2167e7d7539ba7f"));
    QCOMPARE(uuid.size(), 32);
    QVERIFY(!uuid.contains(QLatin1Char('-')));
    QCOMPARE(uuid, mc::offlineUuid(QStringLiteral("Notch")));
}

void TestAuthSession::offlineUuidPlayerLength()
{
    QString uuid = mc::offlineUuid(QStringLiteral("Player"));
    QCOMPARE(uuid.size(), 32);
    QVERIFY(!uuid.contains(QLatin1Char('-')));
}

void TestAuthSession::makeOfflineSessionValid()
{
    mc::AuthSession session = mc::makeOfflineSession(QStringLiteral("Player"));
    QCOMPARE(session.playerName, QStringLiteral("Player"));
    QCOMPARE(session.uuid, mc::offlineUuid(QStringLiteral("Player")));
    QCOMPARE(session.accessToken, QStringLiteral("offline"));
    QCOMPARE(session.session, QStringLiteral("-"));
    QCOMPARE(session.userType, QStringLiteral("offline"));
    QCOMPARE(session.userProperties, QStringLiteral("{}"));
    QVERIFY(!session.demo);
}

void TestAuthSession::makeOfflineSessionInvalid()
{
    mc::AuthSession session = mc::makeOfflineSession(QStringLiteral("has space"));
    QVERIFY(session.playerName.isEmpty());
}

void TestAuthSession::tokenMapKeys()
{
    mc::AuthSession session = mc::makeOfflineSession(QStringLiteral("Player"));
    auto tokens = mc::tokenMap(session, QStringLiteral("1.20.1"), QStringLiteral("1.20.1"),
                               QStringLiteral("release"), QStringLiteral("/games/instance"),
                               QStringLiteral("/games/assets"), QStringLiteral("1.20"));
    QVERIFY(tokens.contains(QStringLiteral("auth_session")));
    QVERIFY(tokens.contains(QStringLiteral("auth_access_token")));
    QVERIFY(tokens.contains(QStringLiteral("auth_player_name")));
    QVERIFY(tokens.contains(QStringLiteral("auth_uuid")));
    QVERIFY(tokens.contains(QStringLiteral("user_properties")));
    QVERIFY(tokens.contains(QStringLiteral("user_type")));
    QVERIFY(tokens.contains(QStringLiteral("profile_name")));
    QVERIFY(tokens.contains(QStringLiteral("version_name")));
    QVERIFY(tokens.contains(QStringLiteral("version_type")));
    QVERIFY(tokens.contains(QStringLiteral("game_directory")));
    QVERIFY(tokens.contains(QStringLiteral("game_assets")));
    QVERIFY(tokens.contains(QStringLiteral("assets_root")));
    QVERIFY(tokens.contains(QStringLiteral("assets_index_name")));
    QCOMPARE(tokens.value(QStringLiteral("auth_player_name")), QStringLiteral("Player"));
    QCOMPARE(tokens.value(QStringLiteral("game_assets")), QStringLiteral("/games/assets"));
    QCOMPARE(tokens.value(QStringLiteral("assets_root")), QStringLiteral("/games/assets"));
}

void TestAuthSession::substituteArgsKnownTokens()
{
    mc::AuthSession session = mc::makeOfflineSession(QStringLiteral("Player"));
    auto tokens = mc::tokenMap(session, QStringLiteral("1.20.1"), QStringLiteral("1.20.1"),
                               QStringLiteral("release"), QStringLiteral("/g"),
                               QStringLiteral("/a"), QStringLiteral("1.20"));
    QStringList args = mc::substituteArgs(
        QStringLiteral("--username ${auth_player_name} --uuid ${auth_uuid}"), tokens);
    QCOMPARE(args, QStringList({"--username", "Player", "--uuid", session.uuid}));
}

void TestAuthSession::substituteArgsUnknownPlaceholder()
{
    mc::AuthSession session = mc::makeOfflineSession(QStringLiteral("Player"));
    auto tokens = mc::tokenMap(session, QStringLiteral("p"), QStringLiteral("v"),
                               QStringLiteral("t"), QStringLiteral("g"), QStringLiteral("a"),
                               QStringLiteral("i"));
    QStringList args = mc::substituteArgs(
        QStringLiteral("--username ${auth_player_name} ${unknown_thing}"), tokens);
    QCOMPARE(args, QStringList({"--username", "Player"}));
    args = mc::substituteArgs(QStringLiteral("${unknown_thing}"), tokens);
    QVERIFY(args.isEmpty());
}

void TestAuthSession::substituteArgsVector()
{
    mc::AuthSession session = mc::makeOfflineSession(QStringLiteral("Player"));
    auto tokens = mc::tokenMap(session, QStringLiteral("1.20.1"), QStringLiteral("1.20.1"),
                               QStringLiteral("release"), QStringLiteral("/g"),
                               QStringLiteral("/a"), QStringLiteral("1.20"));
    QVector<QString> raw{QStringLiteral("--username"), QStringLiteral("${auth_player_name}"),
                         QStringLiteral("--assetIndex"), QStringLiteral("${assets_index_name}"),
                         QStringLiteral("${missing}")};
    QStringList args = mc::substituteArgs(raw, tokens);
    QCOMPARE(args, QStringList({"--username", "Player", "--assetIndex", "1.20"}));
}

QTEST_APPLESS_MAIN(TestAuthSession)
#include "tst_authsession.moc"
