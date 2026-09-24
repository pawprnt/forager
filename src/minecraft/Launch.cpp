#include "minecraft/Launch.h"
#include "minecraft/Instance.h"
#include "minecraft/Assets.h"
#include "minecraft/Classpath.h"
#include "minecraft/JavaDetect.h"
#include "core/Paths.h"
#include "utils/Filesystem.h"
#include "utils/Network.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QProcess>
#include <QSysInfo>
#include <QUrl>

namespace mc {

static QString detectOsName() {
    return QStringLiteral("linux");
}

static QString detectOsArch() {
    return QStringLiteral("linux-") + QSysInfo::currentCpuArchitecture();
}

static std::optional<VersionJson> loadResolvedVersion(const LaunchRequest& req) {
    const auto versionPath = findVersionJson(req.versionsDir, req.versionId);
    if (!versionPath) return std::nullopt;

    const auto version = loadVersionJson(*versionPath);
    if (!version) return std::nullopt;

    if (version->inheritsFrom.isEmpty()) return version;

    const auto mergedJson = resolveInheritance(*version, req.versionsDir);
    if (!mergedJson) return version;
    return parseVersionJson(mergedJson->toUtf8());
}

static int requiredJavaMajor(const VersionJson& version) {
    if (!version.javaVersion.isEmpty()) {
        const int major = version.javaVersion.value(QStringLiteral("majorVersion")).toInt();
        if (major > 0) return major;
    }
    for (const QString& majorStr : version.compatibleJavaMajors) {
        bool ok = false;
        const int major = majorStr.toInt(&ok);
        if (ok && major > 0) return major;
    }
    return 8;
}

static QString pickLibraryUrl(const Library& lib) {
    if (!lib.url.isEmpty()) {
        QString base = lib.url;
        if (!base.endsWith(QLatin1Char('/'))) base += QLatin1Char('/');
        return base;
    }
    return QStringLiteral("https://libraries.minecraft.net/");
}

static bool ensureLibraryJars(const VersionJson& version, const QString& librariesRoot) {
    const QString osName = detectOsName();
    for (const Library& lib : version.libraries) {
        if (!ruleAllows(lib.rules, osName)) continue;
        if (lib.isNative || !lib.natives.isEmpty()) continue;

        const QString path = libraryPath(lib, librariesRoot);
        if (path.isEmpty() || QFile::exists(path)) continue;

        const QString url = pickLibraryUrl(lib) + [&]() {
            if (!lib.artifact.path.isEmpty()) return lib.artifact.path;
            QString rel = path.mid(librariesRoot.size());
            while (rel.startsWith(QLatin1Char('/'))) rel.remove(0, 1);
            return rel;
        }();
        const QByteArray data = net::httpGet(url);
        if (data.isEmpty()) return false;
        if (!fs::writeBytes(path, data)) return false;
    }
    return true;
}

static QJsonObject extractExcludeMap(const VersionJson& version, const QString& osName) {
    QJsonObject exclude;
    for (const Library& lib : version.libraries) {
        if (!ruleAllows(lib.rules, osName)) continue;
        for (auto it = lib.extractExclude.begin(); it != lib.extractExclude.end(); ++it)
            exclude.insert(it.key(), true);
    }
    return exclude;
}

std::optional<LaunchPlan> buildLaunchPlan(const LaunchRequest& req, const AuthSession& session) {
    LaunchPlan plan;

    const auto version = loadResolvedVersion(req);
    if (!version) {
        plan.error = QStringLiteral("version json not found");
        return std::nullopt;
    }

    if (session.playerName.isEmpty()) {
        plan.error = QStringLiteral("invalid username");
        return std::nullopt;
    }

    const int javaMajor = requiredJavaMajor(*version);
    const auto javaPath = ensureJava(javaMajor);
    if (!javaPath) {
        plan.error = QStringLiteral("java %1 not found").arg(javaMajor);
        return std::nullopt;
    }
    plan.javaPath = *javaPath;

    const QString assetsDir = req.assetsRoot;
    const QString indexId = !version->assetIndex.isEmpty()
                                ? version->assetIndex.value(QStringLiteral("id")).toString()
                                : version->assets;
    const AssetsResult assets = ensureAssets(*version, assetsDir, req.librariesRoot);
    if (!assets.ok) plan.error = assets.error;

    QString clientJar;
    QString jarError;
    if (!ensureClientJar(*version, req.librariesRoot, &clientJar, &jarError)) {
        plan.error = jarError;
        return std::nullopt;
    }

    if (!ensureLibraryJars(*version, req.librariesRoot)) {
        plan.error = QStringLiteral("library download failed");
        return std::nullopt;
    }

    const QString osName = detectOsName();
    const QString osArch = detectOsArch();
    const ClasspathResult cp = buildClasspath(*version, req.librariesRoot, osName, osArch);
    if (!cp.error.isEmpty()) {
        plan.error = cp.error;
        return std::nullopt;
    }

    plan.nativesDir = resolveNativesDir(req.instanceDir);
    if (!extractNatives(cp.nativeJars, plan.nativesDir, extractExcludeMap(*version, osName))) {
        plan.error = QStringLiteral("natives extract failed");
        return std::nullopt;
    }

    plan.workingDir = req.gameRoot;
    plan.mainClass = version->mainClass;
    if (plan.mainClass.isEmpty()) {
        plan.error = QStringLiteral("missing mainClass");
        return std::nullopt;
    }

    const QMap<QString, QString> tokens = tokenMap(
        session, session.playerName, version->id,
        version->type.isEmpty() ? QStringLiteral("release") : version->type, req.gameRoot,
        assetsDir, indexId);

    QStringList jvm;
    QStringList game;

    auto appendArgs = [&](const QVector<ArgumentValue>& args, QStringList* out) {
        QVector<QString> raw;
        raw.reserve(args.size());
        for (const ArgumentValue& arg : args) {
            if (!arg.rules.isEmpty() && !ruleAllows(arg.rules, osName)) continue;
            raw.append(arg.value);
        }
        out->append(substituteArgs(raw, tokens));
    };

    if (!version->jvmArgs.isEmpty()) {
        appendArgs(version->jvmArgs, &jvm);
    } else {
        jvm << QStringLiteral("-Djava.library.path=") + plan.nativesDir
            << QStringLiteral("-cp") << cp.jars.join(QLatin1Char(':'));
    }

    if (!version->gameArgs.isEmpty()) {
        appendArgs(version->gameArgs, &game);
    } else if (!version->minecraftArguments.isEmpty()) {
        game = substituteArgs(version->minecraftArguments, tokens);
    }

    if (!jvm.contains(QStringLiteral("-cp")) &&
        !jvm.contains(QStringLiteral("-classpath"))) {
        jvm << QStringLiteral("-cp") << cp.jars.join(QLatin1Char(':'));
    }

    plan.jvmArgs = jvm;
    plan.gameArgs = game;
    return plan;
}

std::unique_ptr<QProcess> execLaunch(const LaunchPlan& plan) {
    if (!plan.error.isEmpty()) return nullptr;

    auto proc = std::make_unique<QProcess>();
    proc->setWorkingDirectory(plan.workingDir);
    proc->setProgram(plan.javaPath);
    proc->setArguments(plan.jvmArgs + QStringList{plan.mainClass} + plan.gameArgs);
    proc->setProcessChannelMode(QProcess::ForwardedChannels);
    proc->start();
    return proc;
}

std::unique_ptr<QProcess> launchInstance(const QString& instanceDir,
                                          const AuthSession& session) {
    LaunchRequest req;
    req.instanceDir = instanceDir;
    req.gameRoot = resolveGameRoot(instanceDir);
    req.versionsDir = QDir(paths::cacheSub(QStringLiteral("minecraft")))
                          .filePath(QStringLiteral("versions"));
    if (!QDir(req.versionsDir).exists()) {
        req.versionsDir = QDir(paths::cacheDir()).filePath(QStringLiteral("versions"));
    }
    if (!QDir(req.versionsDir).exists()) {
        req.versionsDir = QDir(paths::configDir()).filePath(QStringLiteral("versions"));
    }
    req.librariesRoot = paths::cacheSub(QStringLiteral("libraries"));
    req.assetsRoot = paths::cacheSub(QStringLiteral("assets"));

    const auto versionId = detectVersionId(instanceDir);
    if (!versionId) {
        auto proc = std::make_unique<QProcess>();
        proc->setProgram(QStringLiteral("/bin/false"));
        return proc;
    }
    req.versionId = *versionId;

    const auto plan = buildLaunchPlan(req, session);
    if (!plan) return nullptr;
    return execLaunch(*plan);
}

} // namespace mc
