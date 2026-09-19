#include "utils/Subprocess.h"
#include <QProcess>
#include <QEventLoop>
#include <QTimer>
#include <stdexcept>

subprocess::Result subprocess::runChecked(const QString& program, const QStringList& args,
                                          const QString& cwd, int timeoutMs) {
    QProcess proc;
    if (!cwd.isEmpty()) proc.setWorkingDirectory(cwd);
    proc.setProgram(program);
    proc.setArguments(args);

    QEventLoop loop;
    QObject::connect(&proc, &QProcess::finished, &loop, &QEventLoop::quit);
    proc.start();
    QTimer::singleShot(timeoutMs, &loop, &QEventLoop::quit);
    loop.exec();

    Result r;
    r.exitCode = proc.exitCode();
    r.stdout_data = proc.readAllStandardOutput();
    r.stderr_data = proc.readAllStandardError();

    if (r.exitCode != 0) {
        throw std::runtime_error(
            QStringLiteral("%1 failed (exit %2): %3")
                .arg(program).arg(r.exitCode).arg(QString::fromUtf8(r.stderr_data.left(500)))
                .toStdString());
    }
    return r;
}
