#pragma once

#include <QString>
#include <QStringList>
#include <QVector>
#include <optional>

namespace mc {

struct JavaInstall {
    QString path;
    int major = 0;
    QString arch;
    QString vendor;
};

struct PackageOffer {
    QString distro;
    QString packageManager;
    QString jrePackage;
    QString installCmd;
};

std::optional<int> parseJavaMajor(const QString& versionString);
std::optional<JavaInstall> probeJava(const QString& binaryPath);
QVector<JavaInstall> findJavaInstalls();
std::optional<JavaInstall> selectJava(const QVector<JavaInstall>& installs, int requiredMajor);

bool isNixOS();
std::optional<PackageOffer> detectJrePackage(int major);
std::optional<QString> fetchNixJava(int major, const QString& cacheDir);
std::optional<QString> ensureJava(int major);

} // namespace mc
