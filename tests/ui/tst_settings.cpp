#include "ui/dialogs/SettingsDialog.h"
#include "core/Config.h"

#include <QtTest>

class TestSettings : public QObject {
    Q_OBJECT

private slots:
    void constructs();
    void gettersReflectConfig();
};

void TestSettings::constructs()
{
    Config::instance().load();
    SettingsDialog dlg;
    dlg.show();
    QCoreApplication::processEvents();
    QVERIFY(dlg.pages() != nullptr);
    dlg.close();
    QCoreApplication::processEvents();
}

void TestSettings::gettersReflectConfig()
{
    Config::instance().load();
    SettingsDialog dlg;
    QCOMPARE(dlg.gamesDirText(), Config::instance().gamesDir());
    QCOMPARE(dlg.steamAppcacheText(), Config::instance().steamAppcache());
    QVERIFY(!dlg.selectedCardSize().isEmpty());
}

QTEST_MAIN(TestSettings)
#include "tst_settings.moc"
