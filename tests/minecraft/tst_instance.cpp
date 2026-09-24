#include "minecraft/Instance.h"

#include <QtTest>
#include <QDir>
#include <QFile>
#include <QTemporaryDir>

class TestInstance : public QObject {
    Q_OBJECT

private slots:
    void detectVersionFromMmcPack();
    void resolveGameRootPreference();
};

void TestInstance::detectVersionFromMmcPack()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    QFile pack(dir.path() + QStringLiteral("/mmc-pack.json"));
    QVERIFY(pack.open(QIODevice::WriteOnly));
    pack.write(R"({"components":[)");
    pack.write(R"({"uid":"net.minecraft","version":"1.20.1"},)");
    pack.write(R"({"uid":"net.fabricmc.fabric-loader","version":"0.15.11"}]})");
    pack.close();

    auto id = mc::detectVersionId(dir.path());
    QVERIFY(id);
    QCOMPARE(*id, QStringLiteral("1.20.1"));
}

void TestInstance::resolveGameRootPreference()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QCOMPARE(mc::resolveGameRoot(dir.path()), dir.path());

    QVERIFY(QDir(dir.path()).mkpath(QStringLiteral("minecraft")));
    QCOMPARE(mc::resolveGameRoot(dir.path()), dir.path() + QStringLiteral("/minecraft"));

    QVERIFY(QDir(dir.path()).mkpath(QStringLiteral(".minecraft")));
    QCOMPARE(mc::resolveGameRoot(dir.path()), dir.path() + QStringLiteral("/.minecraft"));
}

QTEST_APPLESS_MAIN(TestInstance)
#include "tst_instance.moc"
