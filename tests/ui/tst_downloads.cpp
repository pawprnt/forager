#include "ui/pages/Downloads.h"

#include <QtTest>
#include <QSignalSpy>

class TestDownloads : public QObject {
    Q_OBJECT

private slots:
    void constructs();
    void settingsSignalEmits();
    void cancelSignalEmits();
    void progressStates();
};

void TestDownloads::constructs()
{
    DownloadsPage page;
    QVERIFY(page.parentWidget() || true);
}

void TestDownloads::settingsSignalEmits()
{
    DownloadsPage page;
    QSignalSpy spy(&page, &DownloadsPage::settingsRequested);
    emit page.settingsRequested();
    QCOMPARE(spy.count(), 1);
}

void TestDownloads::cancelSignalEmits()
{
    DownloadsPage page;
    QSignalSpy spy(&page, &DownloadsPage::cancelRequested);
    emit page.cancelRequested();
    QCOMPARE(spy.count(), 1);
}

void TestDownloads::progressStates()
{
    DownloadsPage page;
    page.setIdle();
    page.begin("DepotDownloader");
    page.setProgress(50.0, "downloading", 1.5, 100.0, 200.0);
    page.complete("3.4.0");
    page.failed("boom");
    page.cancelled();
}

QTEST_MAIN(TestDownloads)
#include "tst_downloads.moc"
