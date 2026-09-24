#include "minecraft/JavaDetect.h"

#include "core/Paths.h"
#include "utils/Filesystem.h"
#include "utils/Subprocess.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QRegularExpression>
#include <QSet>
#include <QStringList>

#include <algorithm>

static QString propValue(const QString& text, const QString& key) {
    const QStringList lines = text.split(QLatin1Char('\n'));
    for (const QString& raw : lines) {
        const QString line = raw.trimmed();
        if (!line.startsWith(key)) continue;
        int i = key.size();
        while (i < line.size() && line.at(i).isSpace()) ++i;
        if (i >= line.size() || line.at(i) != QLatin1Char('=')) continue;
        return line.mid(i + 1).trimmed();
    }
    return {};
}

static QString osReleaseValue(const QString& text, const QString& key) {
    const QStringList lines = text.split(QLatin1Char('\n'));
    for (const QString& raw : lines) {
        const QString line = raw.trimmed();
        if (!line.startsWith(key)) continue;
        int i = key.size();
        while (i < line.size() && line.at(i).isSpace()) ++i;
        if (i >= line.size() || line.at(i) != QLatin1Char('=')) continue;
        QString value = line.mid(i + 1).trimmed();
        if (value.size() >= 2) {
            const QChar first = value.at(0);
            const QChar last = value.at(value.size() - 1);
            if ((first == QLatin1Char('"') && last == QLatin1Char('"')) ||
                (first == QLatin1Char('\'') && last == QLatin1Char('\''))) {
                value = value.mid(1, value.size() - 2);
            }
        }
        return value;
    }
    return {};
}

std::optional<int> mc::parseJavaMajor(const QString& versionString) {
    QString v = versionString.trimmed();
    if (v.startsWith(QStringLiteral("1."))) v = v.mid(2);

    int i = 0;
    while (i < v.size() && v.at(i).isDigit()) ++i;
    if (i == 0) return std::nullopt;

    bool ok = false;
    const int major = v.left(i).toInt(&ok);
    if (!ok || major <= 0) return std::nullopt;
    return major;
}

std::optional<mc::JavaInstall> mc::probeJava(const QString& binaryPath) {
    subprocess::Result result;
    try {
        result = subprocess::runChecked(
            binaryPath,
            {QStringLiteral("-XshowSettings:properties"), QStringLiteral("-version")},
            {}, 5000);
    } catch (...) {
        return std::nullopt;
    }

    const QString text = QString::fromUtf8(result.stdout_data) + QLatin1Char('\n') +
                         QString::fromUtf8(result.stderr_data);

    int major = 0;
    const QString spec = propValue(text, QStringLiteral("java.specification.version"));
    if (!spec.isEmpty()) major = parseJavaMajor(spec).value_or(0);

    if (major <= 0) {
        const QString version = propValue(text, QStringLiteral("java.version"));
        if (!version.isEmpty()) major = parseJavaMajor(version).value_or(0);
    }

    if (major <= 0) {
        const QRegularExpression versionLine(QStringLiteral("version \"([^\"]+)\""));
        const QRegularExpressionMatch match = versionLine.match(text);
        if (match.hasMatch()) major = parseJavaMajor(match.captured(1)).value_or(0);
    }

    if (major <= 0) return std::nullopt;

    JavaInstall install;
    install.path = binaryPath;
    install.major = major;
    install.arch = propValue(text, QStringLiteral("os.arch"));
    install.vendor = propValue(text, QStringLiteral("java.vendor"));
    return install;
}

QVector<mc::JavaInstall> mc::findJavaInstalls() {
    QStringList candidates;
    candidates.append(QStringLiteral("java"));

    const QStringList roots = {
        QStringLiteral("/usr/lib/jvm"),
        QStringLiteral("/usr/lib64/jvm"),
        QStringLiteral("/usr/lib32/jvm"),
        QStringLiteral("/usr/java"),
        QStringLiteral("/opt"),
        QStringLiteral("/opt/jdk"),
        QStringLiteral("/opt/jdks"),
        QStringLiteral("/app/jdk"),
    };

    for (const QString& root : roots) {
        QDir dir(root);
        if (!dir.exists()) continue;
        const auto entries = dir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot | QDir::NoSymLinks);
        for (const QFileInfo& entry : entries) {
            const QString base = entry.absoluteFilePath();
            candidates.append(base + QStringLiteral("/bin/java"));
            candidates.append(base + QStringLiteral("/jre/bin/java"));
        }
    }

    const QString envPaths = qEnvironmentVariable("FORAGER_JAVA_PATHS");
    for (const QString& path : envPaths.split(QLatin1Char(':'), Qt::SkipEmptyParts)) {
        const QString trimmedPath = path.trimmed();
        if (!trimmedPath.isEmpty()) candidates.append(trimmedPath);
    }

    QVector<JavaInstall> installs;
    QSet<QString> seen;
    for (const QString& candidate : candidates) {
        const bool bare = candidate == QStringLiteral("java");
        if (!bare && !QFile::exists(candidate)) continue;
        const QString key = QFileInfo(candidate).absoluteFilePath();
        if (seen.contains(key)) continue;
        seen.insert(key);
        auto install = probeJava(candidate);
        if (install) installs.append(*install);
    }

    std::sort(installs.begin(), installs.end(),
              [](const JavaInstall& a, const JavaInstall& b) { return a.major > b.major; });
    return installs;
}

std::optional<mc::JavaInstall> mc::selectJava(const QVector<JavaInstall>& installs,
                                              int requiredMajor) {
    if (installs.isEmpty()) return std::nullopt;

    if (requiredMajor <= 0) {
        const JavaInstall* best = &installs.first();
        for (const JavaInstall& install : installs) {
            if (install.major > best->major) best = &install;
        }
        return *best;
    }

    const JavaInstall* exact = nullptr;
    const JavaInstall* next = nullptr;
    for (const JavaInstall& install : installs) {
        if (install.major == requiredMajor) {
            exact = &install;
            break;
        }
        if (install.major > requiredMajor && (!next || install.major < next->major)) {
            next = &install;
        }
    }
    if (exact) return *exact;
    if (next) return *next;
    return std::nullopt;
}

bool mc::isNixOS() {
    return QFile::exists(QStringLiteral("/etc/NIXOS")) ||
           qEnvironmentVariableIsSet("NIXOS") ||
           QFile::exists(QStringLiteral("/run/current-system"));
}

std::optional<mc::PackageOffer> mc::detectJrePackage(int major) {
    QFile file(QStringLiteral("/etc/os-release"));
    if (!file.open(QIODevice::ReadOnly)) return std::nullopt;

    const QString text = QString::fromUtf8(file.readAll());
    const QString id = osReleaseValue(text, QStringLiteral("ID")).toLower();
    const QString idLike = osReleaseValue(text, QStringLiteral("ID_LIKE")).toLower();

    QStringList tokens = id.split(QLatin1Char(' '), Qt::SkipEmptyParts);
    tokens.append(idLike.split(QLatin1Char(' '), Qt::SkipEmptyParts));

    auto matches = [&tokens](const QStringList& names) {
        for (const QString& token : tokens) {
            for (const QString& name : names) {
                if (token == name || token.startsWith(name + QLatin1Char('-'))) return true;
            }
        }
        return false;
    };

    QString manager;
    if (matches({QStringLiteral("arch"), QStringLiteral("manjaro"), QStringLiteral("endeavouros")}))
        manager = QStringLiteral("pacman");
    else if (matches({QStringLiteral("debian"), QStringLiteral("ubuntu")}))
        manager = QStringLiteral("apt");
    else if (matches({QStringLiteral("fedora")}))
        manager = QStringLiteral("dnf");
    else if (matches({QStringLiteral("opensuse"), QStringLiteral("suse")}))
        manager = QStringLiteral("zypper");
    else if (matches({QStringLiteral("alpine")}))
        manager = QStringLiteral("apk");
    else
        return std::nullopt;

    PackageOffer offer;
    offer.distro = id;
    offer.packageManager = manager;

    if (manager == QStringLiteral("pacman")) {
        if (major == 8)
            offer.jrePackage = QStringLiteral("jre8-openjdk");
        else if (major == 11)
            offer.jrePackage = QStringLiteral("jre11-openjdk");
        else if (major == 17)
            offer.jrePackage = QStringLiteral("jre17-openjdk");
        else if (major == 21)
            offer.jrePackage = QStringLiteral("jre21-openjdk");
        else
            offer.jrePackage = QStringLiteral("jre-openjdk");
        offer.installCmd = QStringLiteral("sudo pacman -S --needed ") + offer.jrePackage;
    } else if (manager == QStringLiteral("apt")) {
        offer.jrePackage = QStringLiteral("openjdk-%1-jre-headless").arg(major);
        offer.installCmd = QStringLiteral("sudo apt install ") + offer.jrePackage;
    } else if (manager == QStringLiteral("dnf") || manager == QStringLiteral("zypper")) {
        offer.jrePackage = QStringLiteral("java-%1-openjdk-headless").arg(major);
        offer.installCmd = manager == QStringLiteral("dnf")
                               ? QStringLiteral("sudo dnf install ") + offer.jrePackage
                               : QStringLiteral("sudo zypper install ") + offer.jrePackage;
    } else {
        if (major == 8)
            offer.jrePackage = QStringLiteral("openjdk8-jre");
        else
            offer.jrePackage = QStringLiteral("openjdk%1-jre-headless").arg(major);
        offer.installCmd = QStringLiteral("sudo apk add ") + offer.jrePackage;
    }

    return offer;
}

std::optional<QString> mc::fetchNixJava(int major, const QString& cacheDir) {
    if (!isNixOS()) return std::nullopt;

    QString attr = QStringLiteral("jdk");
    if (major == 8)
        attr = QStringLiteral("jdk8");
    else if (major == 11)
        attr = QStringLiteral("jdk11");
    else if (major == 17)
        attr = QStringLiteral("jdk17");
    else if (major == 21)
        attr = QStringLiteral("jdk21");

    const QString cacheFile = cacheDir + QStringLiteral("/nix-java-%1.path").arg(major);
    QFile cached(cacheFile);
    if (cached.open(QIODevice::ReadOnly)) {
        const QString storePath = QString::fromUtf8(cached.readAll()).trimmed();
        if (!storePath.isEmpty() && QDir(storePath).exists())
            return storePath + QStringLiteral("/bin/java");
    }

    subprocess::Result result;
    try {
        result = subprocess::runChecked(
            QStringLiteral("nix"),
            {QStringLiteral("build"), QStringLiteral("--no-link"),
             QStringLiteral("--print-out-paths"), QStringLiteral("nixpkgs#") + attr},
            {}, 300000);
    } catch (...) {
        return std::nullopt;
    }

    const QString storePath = QString::fromUtf8(result.stdout_data).trimmed();
    if (storePath.isEmpty() || !QDir(storePath).exists()) return std::nullopt;

    fs::writeBytes(cacheFile, storePath.toUtf8());
    return storePath + QStringLiteral("/bin/java");
}

std::optional<QString> mc::ensureJava(int major) {
    const QVector<JavaInstall> installs = findJavaInstalls();
    if (const auto selected = selectJava(installs, major)) return selected->path;

    if (isNixOS()) {
        const auto javaPath = fetchNixJava(major, paths::cacheSub(QStringLiteral("java")));
        if (javaPath) {
            if (const auto install = probeJava(*javaPath)) return install->path;
        }
        return std::nullopt;
    }

    detectJrePackage(major);
    return std::nullopt;
}
