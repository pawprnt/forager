#include "providers/steam/SteamAppId.h"
#include "core/Config.h"

#include <QtTest>
#include <QTemporaryDir>

class TestSteamAppId : public QObject {
    Q_OBJECT

private slots:
    void init();
    void steamGameUsesOwnAppId();
    void searchTermsPreferSearchNames();
    void searchTermsSeriesCombinedFirst();
    void searchTermsLeafFallback();
    void nameMatchesExact();
    void nameMatchesRejectsShort();

private:
    QTemporaryDir m_dir;
};

void TestSteamAppId::init()
{
    Config::instance().setGamesDir(m_dir.path());
}

void TestSteamAppId::steamGameUsesOwnAppId()
{
    Game g("Portal 2", Source::Steam);
    g.setAppId("620");
    auto id = SteamAppId::resolve(g);
    QVERIFY(id);
    QCOMPARE(*id, QStringLiteral("620"));
}

void TestSteamAppId::searchTermsPreferSearchNames()
{
    Game g("bdcc", Source::Standalone);
    g.setSearchNames({"Broken Dreams Correctional Center"});
    QCOMPARE(SteamAppId::searchTerms(g),
             QStringList{"Broken Dreams Correctional Center"});
}

void TestSteamAppId::searchTermsSeriesCombinedFirst()
{
    Game g("asylum", Source::Standalone);
    g.setPath(m_dir.path() + "/series/sequel/asylum");
    QCOMPARE(SteamAppId::searchTerms(g),
             (QStringList{"sequel asylum", "asylum"}));
}

void TestSteamAppId::searchTermsLeafFallback()
{
    Game g("Hades", Source::Standalone);
    QCOMPARE(SteamAppId::searchTerms(g), QStringList{"Hades"});
}

void TestSteamAppId::nameMatchesExact()
{
    QVERIFY(SteamAppId::nameMatches("Hades", "hades"));
    QVERIFY(SteamAppId::nameMatches("Furry Shades of Gay", "furry shades of gay"));
    QVERIFY(!SteamAppId::nameMatches("Batman: Arkham Asylum", "asylum"));
    QVERIFY(!SteamAppId::nameMatches("NBA 2K26", "26.2"));
    QVERIFY(!SteamAppId::nameMatches("Counter-Strike 2", "2"));
}

void TestSteamAppId::nameMatchesRejectsShort()
{
    QVERIFY(!SteamAppId::nameMatches("2", "Counter-Strike 2"));
    QVERIFY(!SteamAppId::nameMatches("3", "Baldur's Gate 3"));
}

QTEST_APPLESS_MAIN(TestSteamAppId)
#include "tst_steamappid.moc"
