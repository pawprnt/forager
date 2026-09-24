#include "library/Scanner.h"
#include "core/Config.h"

#include <QtTest>
#include <QDir>
#include <QFile>
#include <QTemporaryDir>

class TestScanner : public QObject {
    Q_OBJECT

private slots:
    void init();
    void scanSteam();
    void scanSteamSkipsTools();
    void scanMinecraft();
    void scanStandaloneSeries();
    void scanDrmFreeLayout();

private:
    void writeAcf(const QString& appId, const QString& name);
    QTemporaryDir m_dir;
    QString m_root;
};

static void writeText(const QString& path, const QByteArray& data)
{
    QFile f(path);
    f.open(QIODevice::WriteOnly | QIODevice::Text);
    f.write(data);
    f.close();
}

void TestScanner::init()
{
    m_root = m_dir.path();
    QDir(m_root).removeRecursively();
    QDir().mkpath(m_root);
    Config::instance().setGamesDir(m_root);
}

void TestScanner::writeAcf(const QString& appId, const QString& name)
{
    QString apps = m_root + "/steam/steamapps";
    QDir().mkpath(apps);
    QFile f(apps + "/appmanifest_" + appId + ".acf");
    QVERIFY(f.open(QIODevice::WriteOnly | QIODevice::Text));
    f.write(QStringLiteral("\"appid\"\t\t\"%1\"\n\"name\"\t\t\"%2\"\n")
                .arg(appId, name)
                .toUtf8());
}

static QStringList namesOf(const std::vector<Game>& games)
{
    QStringList out;
    for (const auto& g : games) out << g.name();
    return out;
}

void TestScanner::scanSteam()
{
    writeAcf("70", "Half-Life");
    auto games = scanner::scanAll();
    QCOMPARE(games.size(), size_t(1));
    QCOMPARE(games[0].name(), QStringLiteral("Half-Life"));
    QCOMPARE(games[0].appId(), QStringLiteral("70"));
    QCOMPARE(games[0].source(), Source::Steam);
}

void TestScanner::scanSteamSkipsTools()
{
    writeAcf("239030", "Papers, Please");
    writeAcf("1493710", "Proton Experimental");
    writeAcf("250820", "SteamVR");
    auto games = scanner::scanAll();
    QCOMPARE(namesOf(games), QStringList{"Papers, Please"});
}

void TestScanner::scanMinecraft()
{
    QDir().mkpath(m_root + "/minecraft/foo");
    QDir().mkpath(m_root + "/minecraft/.hidden");
    auto games = scanner::scanAll();
    QCOMPARE(namesOf(games), QStringList{"foo"});
}

void TestScanner::scanStandaloneSeries()
{
    QDir().mkpath(m_root + "/standalone/series/sequel/asylum");
    QDir().mkpath(m_root + "/standalone/series/sequel/blight");
    writeText(m_root + "/standalone/series/sequel/asylum/Game.ini", "[General]\n");
    writeText(m_root + "/standalone/series/sequel/blight/Game.ini", "[General]\n");
    auto games = scanner::scanAll();
    QCOMPARE(namesOf(games), (QStringList{"sequel/asylum", "sequel/blight"}));
}

void TestScanner::scanDrmFreeLayout()
{
    QDir().mkpath(m_root + "/drm-free/standalone/other/bdcc");
    QDir().mkpath(m_root + "/drm-free/standalone/other/kludge");
    QDir().mkpath(m_root + "/drm-free/series/rpgMaker/sequel/asylum");
    QDir().mkpath(m_root + "/drm-free/series/unity/furry shades of gay/2");
    writeText(m_root + "/drm-free/series/rpgMaker/sequel/asylum/Game.ini", "[General]\n");
    writeText(m_root + "/drm-free/series/unity/furry shades of gay/2/Game.exe", "MZ");

    auto games = scanner::scanAll();
    QStringList got = namesOf(games);
    got.sort();
    QCOMPARE(got, (QStringList{"bdcc", "furry shades of gay/2", "kludge", "sequel/asylum"}));
}

QTEST_APPLESS_MAIN(TestScanner)
#include "tst_scanner.moc"
