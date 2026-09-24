#include "ui/MainWindow.h"
#include "ui/pages/Downloads.h"
#include "core/Config.h"

#include <QtTest>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QDir>

class TestMainWindow : public QObject {
    Q_OBJECT

private slots:
    void initTestCase();
    void constructsAndWiresDownloads();

private:
    QTemporaryDir m_gamesDir;
};

void TestMainWindow::initTestCase()
{
    Config::instance().load();
    QVERIFY(m_gamesDir.isValid());
    Config::instance().setGamesDir(m_gamesDir.path());
}

void TestMainWindow::constructsAndWiresDownloads()
{
    MainWindow win;
    QVERIFY(win.grid() != nullptr);
    QVERIFY(win.sidebar() != nullptr);
    QVERIFY(win.gamePage() != nullptr);
    QVERIFY(win.downloadsPage() != nullptr);

    QVERIFY(disconnect(win.downloadsPage(), &DownloadsPage::settingsRequested, &win, nullptr));
    QSignalSpy spy(win.downloadsPage(), &DownloadsPage::settingsRequested);
    emit win.downloadsPage()->settingsRequested();
    QCOMPARE(spy.count(), 1);

    win.close();
    QCoreApplication::processEvents();
    QCoreApplication::processEvents();
}

QTEST_MAIN(TestMainWindow)
#include "tst_mainwindow.moc"
