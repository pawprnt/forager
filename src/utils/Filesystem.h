#pragma once
#include <QString>
#include <QByteArray>

namespace fs {
    bool ensureDir(const QString& path);
    bool ensureParentDir(const QString& filePath);
    bool writeBytes(const QString& filePath, const QByteArray& data);
} // namespace fs
