#pragma once
#include <QString>
#include <QStringList>
#include <QByteArray>
#include <atomic>
#include <functional>

namespace subprocess {
    struct Result {
        int exitCode = -1;
        QByteArray stdout_data;
        QByteArray stderr_data;
    };
    Result runChecked(const QString& program, const QStringList& args = {},
                      const QString& cwd = {}, int timeoutMs = 30000);
    Result runCheckedOptional(const QString& program, const QStringList& args = {},
                              const QString& cwd = {}, int timeoutMs = 30000);

    struct StreamResult {
        bool cancelled = false;
        int exitCode = -1;
        QByteArray stderr_data;
    };
    StreamResult runStreaming(const QString& program, const QStringList& args,
                              std::atomic<bool>* cancel, int readTimeoutMs,
                              const std::function<void(const QByteArray&)>& onOutput);
} // namespace subprocess
