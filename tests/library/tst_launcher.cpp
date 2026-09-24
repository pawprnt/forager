#include "library/Launcher.h"

#include <QtTest>
#include <QFile>
#include <QTemporaryDir>
#include <sys/stat.h>

class TestLauncher : public QObject {
    Q_OBJECT

private slots:
    void findExecutableScript();
    void findExecutableExe();
    void findExecutableNone();
    void launchStandaloneNoPathIsNoop();
    void launchStandaloneNoExecutableIsNoop();
};

void TestLauncher::findExecutableScript()
{
    QTemporaryDir dir;
    QString sh = dir.path() + "/game.sh";
    QFile f(sh);
    QVERIFY(f.open(QIODevice::WriteOnly));
    f.write("#!/bin/sh\nexit 0\n");
    f.close();
    ::chmod(sh.toUtf8().constData(), 0755);
    QCOMPARE(launcher::findExecutable(dir.path()), sh);
}

void TestLauncher::findExecutableExe()
{
    QTemporaryDir dir;
    QString exe = dir.path() + "/Game.exe";
    QFile f(exe);
    QVERIFY(f.open(QIODevice::WriteOnly));
    f.write("MZ");
    f.close();
    QCOMPARE(launcher::findExecutable(dir.path()), exe);
}

void TestLauncher::findExecutableNone()
{
    QVERIFY(launcher::findExecutable("/nonexistent/path").isEmpty());
}

void TestLauncher::launchStandaloneNoPathIsNoop()
{
    Game g("untracked", Source::Standalone);
    QVERIFY(!launcher::launch(g));
}

void TestLauncher::launchStandaloneNoExecutableIsNoop()
{
    QTemporaryDir empty;
    Game g("empty", Source::Standalone);
    g.setPath(empty.path());
    QVERIFY(!launcher::launch(g));
}

QTEST_APPLESS_MAIN(TestLauncher)
#include "tst_launcher.moc"
