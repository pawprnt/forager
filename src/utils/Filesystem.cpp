#include "utils/Filesystem.h"
#include <QDir>

bool fs::ensureDir(const QString& path) {
    return QDir().mkpath(path);
}
