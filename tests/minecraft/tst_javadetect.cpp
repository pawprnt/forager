#include "minecraft/JavaDetect.h"

#include <QtTest>
#include <QFile>

class TestJavaDetect : public QObject {
    Q_OBJECT

private slots:
    void parseJavaMajorVersions();
    void selectJavaExactMajor();
    void selectJavaNextMajor();
    void selectJavaMissingMajor();
    void selectJavaAnyMajor();
    void detectJrePackageOffer();
};

static QVector<mc::JavaInstall> fakeInstalls()
{
    QVector<mc::JavaInstall> installs;
    installs.append({QStringLiteral("/fake/java8"), 8, {}, {}});
    installs.append({QStringLiteral("/fake/java17"), 17, {}, {}});
    installs.append({QStringLiteral("/fake/java21"), 21, {}, {}});
    return installs;
}

void TestJavaDetect::parseJavaMajorVersions()
{
    auto eight = mc::parseJavaMajor(QStringLiteral("1.8.0_22"));
    QVERIFY(eight);
    QCOMPARE(*eight, 8);

    auto seventeen = mc::parseJavaMajor(QStringLiteral("17.0.9"));
    QVERIFY(seventeen);
    QCOMPARE(*seventeen, 17);

    auto seven = mc::parseJavaMajor(QStringLiteral("1.7.0_80"));
    QVERIFY(seven);
    QCOMPARE(*seven, 7);

    QVERIFY(!mc::parseJavaMajor(QStringLiteral("garbage")));
}

void TestJavaDetect::selectJavaExactMajor()
{
    auto selected = mc::selectJava(fakeInstalls(), 17);
    QVERIFY(selected);
    QCOMPARE(selected->major, 17);
}

void TestJavaDetect::selectJavaNextMajor()
{
    auto selected = mc::selectJava(fakeInstalls(), 16);
    QVERIFY(selected);
    QCOMPARE(selected->major, 17);
}

void TestJavaDetect::selectJavaMissingMajor()
{
    QVERIFY(!mc::selectJava(fakeInstalls(), 25));
}

void TestJavaDetect::selectJavaAnyMajor()
{
    auto selected = mc::selectJava(fakeInstalls(), 0);
    QVERIFY(selected);
    QCOMPARE(selected->major, 21);
}

void TestJavaDetect::detectJrePackageOffer()
{
    if (!QFile::exists(QStringLiteral("/etc/os-release")))
        QSKIP("no /etc/os-release");

    auto offer = mc::detectJrePackage(17);
    if (!offer) return;
    QVERIFY(!offer->packageManager.isEmpty());
    QVERIFY(!offer->jrePackage.isEmpty());
    QVERIFY(!offer->installCmd.isEmpty());
}

QTEST_APPLESS_MAIN(TestJavaDetect)
#include "tst_javadetect.moc"
