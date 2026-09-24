#include "utils/Json.h"

#include <QtTest>
#include <QTemporaryDir>

class TestJson : public QObject {
    Q_OBJECT

private slots:
    void parseObjectValid();
    void parseObjectInvalid();
    void writeAndReadRoundtrip();
    void readMissingFile();

private:
    QTemporaryDir m_dir;
};

void TestJson::parseObjectValid()
{
    auto obj = json::parseObject(R"({"a":1,"b":"x"})");
    QVERIFY(obj);
    QCOMPARE((*obj)["a"].toInt(), 1);
    QCOMPARE((*obj)["b"].toString(), QStringLiteral("x"));
}

void TestJson::parseObjectInvalid()
{
    QVERIFY(!json::parseObject("not json").has_value());
    QVERIFY(!json::parseObject("[1,2]").has_value());
}

void TestJson::writeAndReadRoundtrip()
{
    QString path = m_dir.path() + "/nested/out.json";
    QJsonObject out;
    out["k"] = "v";
    out["n"] = 42;
    QVERIFY(json::writeObject(path, out));

    auto in = json::readObject(path);
    QVERIFY(in);
    QCOMPARE((*in)["k"].toString(), QStringLiteral("v"));
    QCOMPARE((*in)["n"].toInt(), 42);
}

void TestJson::readMissingFile()
{
    QVERIFY(!json::readObject(m_dir.path() + "/missing.json").has_value());
}

QTEST_APPLESS_MAIN(TestJson)
#include "tst_json.moc"
