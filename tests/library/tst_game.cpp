#include "core/Game.h"
#include "core/Config.h"
#include "core/Paths.h"

#include <QtTest>
#include <QTemporaryDir>

class TestGame : public QObject {
    Q_OBJECT

private slots:
    void init();
    void searchLeafNotUsed();
    void searchSkipsGenericContainer();
    void engineSingleGameSearchesOwnName();
    void directSingleGameNoSearch();
    void searchNamesWins();
    void steamNeverSearches();
    void outsideGamesDirNoSearch();

private:
    QTemporaryDir m_dir;
    QString m_root;
};

void TestGame::init()
{
    m_root = m_dir.path();
    Config::instance().setGamesDir(m_root);
}

void TestGame::searchLeafNotUsed()
{
    Game g("series/sequel/asylum", Source::Standalone);
    g.setPath(m_root + "/standalone/series/sequel/asylum");
    auto plan = g.sgdbSearch();
    QVERIFY(plan);
    QCOMPARE(plan->first, QStringList{"sequel"});
    QCOMPARE(plan->second, QStringLiteral("asylum"));
}

void TestGame::searchSkipsGenericContainer()
{
    Game g("26.2", Source::Standalone);
    g.setPath(m_root + "/minecraft/26.2");
    QVERIFY(!g.sgdbSearch().has_value());
}

void TestGame::engineSingleGameSearchesOwnName()
{
    Game g("monster girl quest", Source::Standalone);
    g.setPath(m_root + "/standalone/other/monster girl quest");
    auto plan = g.sgdbSearch();
    QVERIFY(plan);
    QCOMPARE(plan->first, QStringList{"monster girl quest"});
    QVERIFY(plan->second.isEmpty());
}

void TestGame::directSingleGameNoSearch()
{
    Game g("Hades", Source::Standalone);
    g.setPath(m_root + "/standalone/Hades");
    QVERIFY(!g.sgdbSearch().has_value());
}

void TestGame::searchNamesWins()
{
    Game g("bdcc", Source::Standalone);
    g.setPath(m_root + "/standalone/bdcc");
    g.setSearchNames({"Broken Dreams Correctional Center"});
    auto plan = g.sgdbSearch();
    QVERIFY(plan);
    QCOMPARE(plan->first, QStringList{"Broken Dreams Correctional Center"});
    QVERIFY(plan->second.isEmpty());
}

void TestGame::steamNeverSearches()
{
    Game g("Foo", Source::Steam);
    g.setPath(m_root + "/steam/steamapps/common/Foo");
    g.setAppId("123");
    QVERIFY(!g.sgdbSearch().has_value());
}

void TestGame::outsideGamesDirNoSearch()
{
    Game g("elsewhere", Source::Standalone);
    g.setPath("/tmp/elsewhere");
    QVERIFY(!g.sgdbSearch().has_value());
}

QTEST_APPLESS_MAIN(TestGame)
#include "tst_game.moc"
