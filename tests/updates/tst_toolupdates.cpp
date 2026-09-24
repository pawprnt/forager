#include "updates/ToolUpdates.h"

#include <QtTest>

class TestToolUpdates : public QObject {
    Q_OBJECT

private slots:
    void checkReturnsEmptyStub();
    void updateReturnsEmptyStub();
};

void TestToolUpdates::checkReturnsEmptyStub()
{
    QVERIFY(toolupdates::checkToolUpdates().isEmpty());
}

void TestToolUpdates::updateReturnsEmptyStub()
{
    QVERIFY(toolupdates::updateToolUpdates().isEmpty());
}

QTEST_APPLESS_MAIN(TestToolUpdates)
#include "tst_toolupdates.moc"
