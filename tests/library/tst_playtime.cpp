#include "library/Playtime.h"

#include <QtTest>
#include <QTemporaryDir>

class TestPlaytime : public QObject {
    Q_OBJECT

private slots:
    void gameKeyUsesAppId();
    void gameKeyWithoutPathOrAppId();
    void formatPlaytimeUnits();
    void storeAddAndRead();
    void storeRoundtrip();

private:
    QTemporaryDir m_dir;
};

void TestPlaytime::gameKeyUsesAppId()
{
    Game g("X", Source::Steam);
    g.setAppId("440");
    QCOMPARE(PlaytimeStore::gameKey(g), QStringLiteral("steam:440"));
}

void TestPlaytime::gameKeyWithoutPathOrAppId()
{
    Game g("Mystery", Source::Standalone);
    g.setInstalled(false);
    QString key = PlaytimeStore::gameKey(g);
    QVERIFY(key.contains("Mystery"));
}

void TestPlaytime::formatPlaytimeUnits()
{
    QCOMPARE(PlaytimeStore::formatPlaytime(45), QStringLiteral("45 s"));
    QCOMPARE(PlaytimeStore::formatPlaytime(600), QStringLiteral("10 min"));
    QCOMPARE(PlaytimeStore::formatPlaytime(9000), QStringLiteral("2.5 h"));
}

void TestPlaytime::storeAddAndRead()
{
    QString path = m_dir.path() + "/playtime.json";
    PlaytimeStore store(path);
    store.add("steam:440", 10.0f);
    store.add("steam:440", 5.0f);
    store.add("steam:440", -1.0f);
    QCOMPARE(store.playtime("steam:440"), 15.0f);
    store.save();

    PlaytimeStore reloaded(path);
    QCOMPARE(reloaded.playtime("steam:440"), 15.0f);
}

void TestPlaytime::storeRoundtrip()
{
    QString path = m_dir.path() + "/rt.json";
    {
        PlaytimeStore store(path);
        store.touch("standalone:/games/foo");
        store.save();
    }
    PlaytimeStore store(path);
    QVERIFY(store.lastPlayed("standalone:/games/foo") > 0.0f);
    QCOMPARE(store.playtime("standalone:/games/foo"), 0.0f);
}

QTEST_APPLESS_MAIN(TestPlaytime)
#include "tst_playtime.moc"
