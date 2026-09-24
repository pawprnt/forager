#include "utils/Acf.h"

#include <QtTest>
#include <QDir>
#include <QFile>
#include <QTemporaryDir>

class TestAcf : public QObject {
    Q_OBJECT

private slots:
    void parseAppIdAndName();
    void parseMissingFile();
    void listManifests();

private:
    QTemporaryDir m_dir;
};

static void writeText(const QString& path, const QByteArray& data)
{
    QFile f(path);
    f.open(QIODevice::WriteOnly | QIODevice::Text);
    f.write(data);
    f.close();
}

void TestAcf::parseAppIdAndName()
{
    QString path = m_dir.path() + "/appmanifest_70.acf";
    QFile f(path);
    QVERIFY(f.open(QIODevice::WriteOnly | QIODevice::Text));
    f.write("\"appid\"\t\t\"70\"\n\"name\"\t\t\"Half-Life\"\n");
    f.close();

    auto [appId, name] = acf::parseFile(path);
    QCOMPARE(appId, QStringLiteral("70"));
    QCOMPARE(name, QStringLiteral("Half-Life"));
}

void TestAcf::parseMissingFile()
{
    auto [appId, name] = acf::parseFile(m_dir.path() + "/nope.acf");
    QVERIFY(appId.isEmpty());
    QVERIFY(name.isEmpty());
}

void TestAcf::listManifests()
{
    QDir().mkpath(m_dir.path() + "/steamapps");
    writeText(m_dir.path() + "/steamapps/appmanifest_70.acf", "\"appid\"\t\t\"70\"\n");
    writeText(m_dir.path() + "/steamapps/readme.txt", "hi");

    QStringList manifests = acf::listManifests(m_dir.path() + "/steamapps");
    QCOMPARE(manifests.size(), 1);
    QVERIFY(manifests.first().endsWith("appmanifest_70.acf"));

    QVERIFY(acf::listManifests(m_dir.path() + "/missing").isEmpty());
}

QTEST_APPLESS_MAIN(TestAcf)
#include "tst_acf.moc"
