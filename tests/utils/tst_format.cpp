#include "utils/Format.h"
#include "library/Playtime.h"

#include <QtTest>

class TestFormat : public QObject {
    Q_OBJECT

private slots:
    void sizeBytes();
    void sizeKilobytes();
    void sizeMegabytes();
    void playtimeSeconds();
    void playtimeMinutes();
    void playtimeHours();
};

void TestFormat::sizeBytes()
{
    QCOMPARE(format::size(0), QStringLiteral("0.0 B"));
    QCOMPARE(format::size(512), QStringLiteral("512.0 B"));
}

void TestFormat::sizeKilobytes()
{
    QCOMPARE(format::size(2048), QStringLiteral("2.0 KB"));
}

void TestFormat::sizeMegabytes()
{
    QCOMPARE(format::size(5LL * 1024 * 1024), QStringLiteral("5.0 MB"));
}

void TestFormat::playtimeSeconds()
{
    QCOMPARE(PlaytimeStore::formatPlaytime(45), QStringLiteral("45 s"));
}

void TestFormat::playtimeMinutes()
{
    QCOMPARE(PlaytimeStore::formatPlaytime(600), QStringLiteral("10 min"));
}

void TestFormat::playtimeHours()
{
    QCOMPARE(PlaytimeStore::formatPlaytime(9000), QStringLiteral("2.5 h"));
}

QTEST_APPLESS_MAIN(TestFormat)
#include "tst_format.moc"
