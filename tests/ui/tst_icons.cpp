#include "ui/Icons.h"

#include <QtTest>
#include <QFileInfo>
#include <QDir>

static const char* BUNDLED[] = {
    "settings", "arrow-left", "arrow-right", "box", "play", "stop",
    "floppy-disk", "xmark", "clock-rotate-right", "download", "folder",
    "shield", "user",
};

class TestIcons : public QObject {
    Q_OBJECT

private slots:
    void allBundledIconsExistOnDisk();
    void missingIconIsNull();
};

void TestIcons::allBundledIconsExistOnDisk()
{
    QString srcIcons = QStringLiteral(SOURCE_DIR) + "/src/forager/assets/icons";
    for (const char* name : BUNDLED) {
        QString path = srcIcons + "/" + name + ".svg";
        QVERIFY2(QFileInfo::exists(path), qPrintable(path));
    }
}

void TestIcons::missingIconIsNull()
{
    QVERIFY(icons::loadIcon("definitely-not-a-real-icon").isNull());
}

QTEST_MAIN(TestIcons)
#include "tst_icons.moc"
