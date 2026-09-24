#include "ui/pages/Store.h"

#include <QtTest>

class TestStore : public QObject {
    Q_OBJECT

private slots:
    void recolorJsInjectsStyle();
    void constructs();
    void steamTabSwitches();
};

void TestStore::recolorJsInjectsStyle()
{
    QString js = StorePage::steamRecolorJs();
    QVERIFY(js.contains("spacetheme-css"));
    QVERIFY(js.contains("createElement"));
}

void TestStore::constructs()
{
    StorePage page;
    QVERIFY(page.stack() != nullptr);
    QCOMPARE(page.stack()->count(), 4);
    page.deleteLater();
}

void TestStore::steamTabSwitches()
{
    StorePage page;
    QVERIFY(page.tabsGroup() != nullptr);
    auto* btn = page.tabsGroup()->button(0);
    QVERIFY(btn);
    btn->click();
    QCoreApplication::processEvents();
    QCOMPARE(page.stack()->currentIndex(), 0);
    page.deleteLater();
}

QTEST_MAIN(TestStore)
#include "tst_store.moc"
