#include "minecraft/Classpath.h"

#include "utils/Filesystem.h"

#include <QDir>
#include <QFileInfo>
#include <QProcess>
#include <QStandardPaths>

static QString classifierName(const QString& name, const QString& classifier) {
    QString base = name;
    QString suffix;
    const int at = base.indexOf(QLatin1Char('@'));
    if (at >= 0) {
        suffix = base.mid(at);
        base = base.left(at);
    }

    const QStringList parts = base.split(QLatin1Char(':'));
    if (parts.size() != 3) return name;
    return parts.at(0) + QStringLiteral(":") + parts.at(1) + QStringLiteral(":") +
           parts.at(2) + QStringLiteral(":") + classifier + suffix;
}

static QString nativeJarPath(const mc::Library& lib, const QString& librariesRoot,
                             const QString& osArch) {
    const QString classifier = mc::nativeClassifier(lib, osArch);
    if (classifier.isEmpty()) return {};

    const QString suffix = classifier + QStringLiteral(".jar");
    if (lib.artifact.path.endsWith(suffix))
        return QDir(librariesRoot).filePath(lib.artifact.path);
    for (const mc::LibraryDownloads& download : lib.classifiers) {
        if (download.path.endsWith(suffix))
            return QDir(librariesRoot).filePath(download.path);
    }

    mc::Library withClassifier = lib;
    withClassifier.artifact.path.clear();
    withClassifier.name = classifierName(lib.name, classifier);
    return mc::libraryPath(withClassifier, librariesRoot);
}

namespace mc {

ClasspathResult buildClasspath(const VersionJson& version, const QString& librariesRoot,
                               const QString& osName, const QString& osArch) {
    ClasspathResult result;
    for (const Library& lib : version.libraries) {
        if (!ruleAllows(lib.rules, osName)) continue;
        if (lib.isNative || !lib.natives.isEmpty()) {
            const QString path = nativeJarPath(lib, librariesRoot, osArch);
            if (!path.isEmpty()) result.nativeJars.append(path);
        } else {
            const QString path = libraryPath(lib, librariesRoot);
            if (!path.isEmpty()) result.jars.append(path);
        }
    }

    if (version.downloads.contains(QStringLiteral("client"))) {
        const QJsonObject client = version.downloads.value(QStringLiteral("client")).toObject();
        QString rel = client.value(QStringLiteral("path")).toString();
        if (rel.isEmpty())
            rel = QStringLiteral("com/mojang/minecraft/%1/minecraft-%1-client.jar")
                      .arg(version.id);
        result.jars.append(QDir(librariesRoot).filePath(rel));
    }
    return result;
}

bool extractNatives(const QStringList& nativeJars, const QString& destDir,
                    const QJsonObject& extractExclude) {
    if (nativeJars.isEmpty()) return true;

    const QString unzip = QStandardPaths::findExecutable(QStringLiteral("unzip"));
    if (unzip.isEmpty()) return false;
    if (!fs::ensureDir(destDir)) return false;

    for (const QString& jar : nativeJars) {
        if (!QFileInfo::exists(jar)) return false;

        QStringList args{QStringLiteral("-o"), QStringLiteral("-q"), jar};
        for (auto it = extractExclude.begin(); it != extractExclude.end(); ++it) {
            args << QStringLiteral("-x") << it.key() + QStringLiteral("*");
        }
        args << QStringLiteral("-d") << destDir;

        QProcess proc;
        proc.start(unzip, args);
        if (!proc.waitForFinished(-1)) return false;
        if (proc.exitStatus() != QProcess::NormalExit || proc.exitCode() != 0) return false;
    }
    return true;
}

} // namespace mc
