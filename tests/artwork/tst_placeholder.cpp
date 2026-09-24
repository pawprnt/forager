#include "artwork/Placeholder.h"
#include "ui/Fonts.h"

#include <QtTest>

class TestPlaceholder : public QObject {
    Q_OBJECT

private slots:
    void initTestCase();
    void placeholderGridNonNull();
    void placeholderCardNonNull();
    void placeholderGridHasSize();

private:
};

void TestPlaceholder::initTestCase()
{
    fonts::registerFonts();
}

void TestPlaceholder::placeholderGridNonNull()
{
    QPixmap pix = placeholder::placeholderGrid("No Art 2", 165, 248);
    QVERIFY(!pix.isNull());
    QCOMPARE(pix.width(), 165);
    QCOMPARE(pix.height(), 248);
}

void TestPlaceholder::placeholderCardNonNull()
{
    QPixmap pix = placeholder::placeholderCard("No Art 1", 900, 420);
    QVERIFY(!pix.isNull());
    QCOMPARE(pix.width(), 900);
    QCOMPARE(pix.height(), 420);
}

void TestPlaceholder::placeholderGridHasSize()
{
    QPixmap pix = placeholder::placeholderGrid("Rando Game 3", 165, 248);
    QImage img = pix.toImage();
    QVERIFY(img.width() > 0);
    QVERIFY(img.height() > 0);
}

QTEST_MAIN(TestPlaceholder)
#include "tst_placeholder.moc"
