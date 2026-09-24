#include "utils/Subprocess.h"
#include <QProcess>
#include <stdexcept>

subprocess::Result subprocess::runCheckedOptional(const QString& program, const QStringList& args,
                                                  const QString& cwd, int timeoutMs) {
    QProcess proc;
    if (!cwd.isEmpty()) proc.setWorkingDirectory(cwd);
    proc.setProgram(program);
    proc.setArguments(args);
    proc.start();
    proc.waitForFinished(timeoutMs);

    Result r;
    r.exitCode = proc.exitCode();
    r.stdout_data = proc.readAllStandardOutput();
    r.stderr_data = proc.readAllStandardError();
    return r;
}

subprocess::Result subprocess::runChecked(const QString& program, const QStringList& args,
                                          const QString& cwd, int timeoutMs) {
    Result r = runCheckedOptional(program, args, cwd, timeoutMs);

    if (r.exitCode != 0) {
        throw std::runtime_error(
            QStringLiteral("%1 failed (exit %2): %3")
                .arg(program).arg(r.exitCode).arg(QString::fromUtf8(r.stderr_data.left(500)))
                .toStdString());
    }
    return r;
}

subprocess::StreamResult subprocess::runStreaming(const QString& program, const QStringList& args,
                                                  std::atomic<bool>* cancel, int readTimeoutMs,
                                                  const std::function<void(const QByteArray&)>& onOutput) {
    QProcess proc;
    proc.setProgram(program);
    proc.setArguments(args);
    proc.start();

    while (proc.state() != QProcess::NotRunning) {
        if (cancel && cancel->load()) {
            proc.kill();
            return {true, -1, {}};
        }
        proc.waitForReadyRead(readTimeoutMs);
        if (onOutput) onOutput(proc.readAllStandardOutput());
    }

    return {false, proc.exitCode(), proc.readAllStandardError()};
}
