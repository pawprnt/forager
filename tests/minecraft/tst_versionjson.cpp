#include "minecraft/VersionJson.h"

#include <QtTest>

class TestVersionJson : public QObject {
    Q_OBJECT

private slots:
    void parseModernVersion();
    void parseLegacyMinecraftArguments();
    void libraryPathBrigadier();
};

void TestVersionJson::parseModernVersion()
{
    const QByteArray data = R"({
        "id": "1.20.1",
        "mainClass": "net.minecraft.client.main.Main",
        "type": "release",
        "assets": "1.20",
        "minimumLauncherVersion": 18,
        "javaVersion": {"component": "java-runtime-gamma", "majorVersion": 17},
        "arguments": {
            "game": [
                "--demo",
                {"rules": [{"action": "allow", "os": {"name": "osx"}}],
                 "value": "--fixed"}
            ],
            "jvm": ["-Djava.library.path=${natives_directory}"]
        },
        "libraries": [{"name": "com.mojang:brigadier:1.0.18"}]
    })";

    auto version = mc::parseVersionJson(data);
    QVERIFY(version);
    QCOMPARE(version->id, QStringLiteral("1.20.1"));
    QCOMPARE(version->mainClass, QStringLiteral("net.minecraft.client.main.Main"));
    QCOMPARE(version->type, QStringLiteral("release"));
    QCOMPARE(version->assets, QStringLiteral("1.20"));
    QCOMPARE(version->minimumLauncherVersion, 18);
    QCOMPARE(version->javaVersion.value(QStringLiteral("majorVersion")).toInt(), 17);
    QCOMPARE(version->gameArgs.size(), 2);
    QCOMPARE(version->gameArgs.at(0).value, QStringLiteral("--demo"));
    QVERIFY(version->gameArgs.at(0).rules.isEmpty());
    QCOMPARE(version->gameArgs.at(1).value, QStringLiteral("--fixed"));
    QCOMPARE(version->gameArgs.at(1).rules.size(), 1);
    QCOMPARE(version->gameArgs.at(1).rules.at(0).osName, QStringLiteral("osx"));
    QCOMPARE(version->jvmArgs.size(), 1);
    QCOMPARE(version->jvmArgs.at(0).value,
             QStringLiteral("-Djava.library.path=${natives_directory}"));
    QCOMPARE(version->libraries.size(), 1);
    QCOMPARE(version->libraries.at(0).name, QStringLiteral("com.mojang:brigadier:1.0.18"));
}

void TestVersionJson::parseLegacyMinecraftArguments()
{
    const QByteArray data = R"({
        "id": "1.9",
        "mainClass": "net.minecraft.client.main.Main",
        "minecraftArguments": "--username ${auth_player_name} --version ${version_name}",
        "libraries": []
    })";

    auto version = mc::parseVersionJson(data);
    QVERIFY(version);
    QCOMPARE(version->id, QStringLiteral("1.9"));
    QCOMPARE(version->minecraftArguments,
             QStringLiteral("--username ${auth_player_name} --version ${version_name}"));
    QVERIFY(version->gameArgs.isEmpty());
    QVERIFY(version->jvmArgs.isEmpty());
}

void TestVersionJson::libraryPathBrigadier()
{
    mc::Library lib;
    lib.name = QStringLiteral("com.mojang:brigadier:1.0.18");
    QCOMPARE(mc::libraryPath(lib, QStringLiteral("/libraries")),
             QStringLiteral("/libraries/com/mojang/brigadier/1.0.18/brigadier-1.0.18.jar"));
}

QTEST_APPLESS_MAIN(TestVersionJson)
#include "tst_versionjson.moc"
