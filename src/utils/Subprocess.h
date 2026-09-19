#pragma once
#include <QString>
#include <QStringList>
#include <QByteArray>

namespace subprocess {
    struct Result {
        int exitCode = -1;
        QByteArray stdout_data;
        QByteArray stderr_data;
    };
    Result runChecked(const QString& program, const QStringList& args = {},
                      const QString& cwd = {}, int timeoutMs = 30000);
} // namespace subprocess
