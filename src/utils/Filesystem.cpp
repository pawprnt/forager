#include "utils/Filesystem.h"
#include <QDir>
#include <QFileInfo>
#include <QFile>

bool fs::ensureDir(const QString& path) {
    return QDir().mkpath(path);
}

bool fs::ensureParentDir(const QString& filePath) {
    return QDir().mkpath(QFileInfo(filePath).absolutePath());
}

bool fs::writeBytes(const QString& filePath, const QByteArray& data) {
    if (!ensureParentDir(filePath)) return false;
    QFile f(filePath);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate)) return false;
    return f.write(data) == data.size();
}
