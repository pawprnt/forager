#include "minecraft/Assets.h"

#include "utils/Filesystem.h"
#include "utils/Json.h"

#include <QtTest>
#include <QJsonObject>
#include <QTemporaryDir>

class TestAssets : public QObject {
    Q_OBJECT

private slots:
    void ensureClientJarPreexistingFile();
    void ensureClientJarMissingFails();
    void ensureAssetsWithoutIndexFails();
    void ensureAssetsPresentObjects();

private:
    QTemporaryDir m_dir;
};

void TestAssets::ensureClientJarPreexistingFile()
{
    mc::VersionJson version;
    version.id = QStringLiteral("1.20.1");
    QJsonObject client;
    client[QStringLiteral("path")] =
        QStringLiteral("com/mojang/minecraft/1.20.1/minecraft-1.20.1-client.jar");
    client[QStringLiteral("url")] = QString();
    version.downloads[QStringLiteral("client")] = client;

    const QString jarPath = m_dir.path() + "/" + client[QStringLiteral("path")].toString();
    QVERIFY(fs::writeBytes(jarPath, QByteArrayLiteral("jar-bytes")));

    QString out;
    QString err;
    QVERIFY(mc::ensureClientJar(version, m_dir.path(), &out, &err));
    QCOMPARE(out, jarPath);
    QVERIFY(err.isEmpty());
}

void TestAssets::ensureClientJarMissingFails()
{
    mc::VersionJson version;
    version.id = QStringLiteral("1.20.1");
    QJsonObject client;
    client[QStringLiteral("path")] =
        QStringLiteral("com/mojang/minecraft/1.20.1/minecraft-1.20.1-client.jar");
    client[QStringLiteral("url")] = QString();
    version.downloads[QStringLiteral("client")] = client;

    QString out;
    QString err;
    const QString libs = m_dir.path() + "/missing-libs";
    QVERIFY(!mc::ensureClientJar(version, libs, &out, &err));
    QVERIFY(out.isEmpty());
    QVERIFY(!err.isEmpty());
}

void TestAssets::ensureAssetsWithoutIndexFails()
{
    mc::VersionJson version;
    const mc::AssetsResult result =
        mc::ensureAssets(version, m_dir.path() + "/assets", m_dir.path() + "/libraries");
    QVERIFY(!result.ok);
    QCOMPARE(result.error, QStringLiteral("no asset index"));
}

void TestAssets::ensureAssetsPresentObjects()
{
    const QString assetsDir = m_dir.path() + "/assets-present";
    const QString hash = QStringLiteral("0123456789abcdef0123456789abcdef01234567");
    const QByteArray objectData = QByteArrayLiteral("object-bytes");

    QJsonObject entry;
    entry[QStringLiteral("hash")] = hash;
    entry[QStringLiteral("size")] = static_cast<int>(objectData.size());
    QJsonObject objects;
    objects[QStringLiteral("minecraft/sounds/test.ogg")] = entry;
    QJsonObject index;
    index[QStringLiteral("objects")] = objects;

    QVERIFY(json::writeObject(assetsDir + "/indexes/present.json", index));
    QVERIFY(fs::writeBytes(assetsDir + "/objects/" + hash.left(2) + "/" + hash, objectData));

    mc::VersionJson version;
    version.assets = QStringLiteral("present");
    version.assetIndex = QJsonObject{{QStringLiteral("id"), QStringLiteral("present")}};

    const mc::AssetsResult result =
        mc::ensureAssets(version, assetsDir, m_dir.path() + "/libraries");
    QVERIFY(result.ok);
    QVERIFY(result.error.isEmpty());
    QCOMPARE(result.indexId, QStringLiteral("present"));
    QCOMPARE(result.assetsDir, assetsDir);
}

QTEST_APPLESS_MAIN(TestAssets)
#include "tst_assets.moc"
