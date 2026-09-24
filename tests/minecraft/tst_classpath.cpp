#include "minecraft/Classpath.h"

#include <QtTest>

class TestClasspath : public QObject {
    Q_OBJECT

private slots:
    void emptyRulesAllow();
    void disallowWindowsOnLinux();
    void allowOnlyLinux();
};

void TestClasspath::emptyRulesAllow()
{
    QVERIFY(mc::ruleAllows({}, QStringLiteral("linux")));
    QVERIFY(mc::ruleAllows({}, QStringLiteral("windows")));
    QVERIFY(mc::ruleAllows({}, QStringLiteral("osx")));
}

void TestClasspath::disallowWindowsOnLinux()
{
    QVector<mc::LibraryRule> rules;
    rules.append({QStringLiteral("allow"), QString()});
    rules.append({QStringLiteral("disallow"), QStringLiteral("windows")});

    QVERIFY(mc::ruleAllows(rules, QStringLiteral("linux")));
    QVERIFY(mc::ruleAllows(rules, QStringLiteral("osx")));
    QVERIFY(!mc::ruleAllows(rules, QStringLiteral("windows")));
}

void TestClasspath::allowOnlyLinux()
{
    QVector<mc::LibraryRule> rules;
    rules.append({QStringLiteral("allow"), QStringLiteral("linux")});

    QVERIFY(mc::ruleAllows(rules, QStringLiteral("linux")));
    QVERIFY(!mc::ruleAllows(rules, QStringLiteral("windows")));
    QVERIFY(!mc::ruleAllows(rules, QStringLiteral("osx")));
}

QTEST_APPLESS_MAIN(TestClasspath)
#include "tst_classpath.moc"
