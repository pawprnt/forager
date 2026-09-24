#pragma once
#include <QString>
#include <QStringList>
#include <utility>

namespace acf {
    std::pair<QString, QString> parseFile(const QString& filePath);
    QStringList listManifests(const QString& steamappsDir);
} // namespace acf
