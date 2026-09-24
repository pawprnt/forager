#include "minecraft/VersionJson.h"

#include "utils/Json.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QSet>

static QVector<mc::LibraryRule> parseRules(const QJsonValue& val) {
    QVector<mc::LibraryRule> rules;
    if (!val.isArray()) return rules;

    for (const QJsonValue& entry : val.toArray()) {
        if (!entry.isObject()) continue;
        const QJsonObject obj = entry.toObject();
        const QString action = obj.value(QStringLiteral("action")).toString();
        if (action != QLatin1String("allow") && action != QLatin1String("disallow"))
            continue;

        mc::LibraryRule rule;
        rule.action = action;
        const QJsonValue os = obj.value(QStringLiteral("os"));
        if (os.isObject()) {
            const QJsonValue name = os.toObject().value(QStringLiteral("name"));
            if (!name.isString()) continue;
            rule.osName = name.toString();
        }
        rules.append(rule);
    }
    return rules;
}

static mc::LibraryDownloads parseDownload(const QJsonObject& obj) {
    mc::LibraryDownloads download;
    download.path = obj.value(QStringLiteral("path")).toString();
    download.url = obj.value(QStringLiteral("url")).toString();
    download.sha1 = obj.value(QStringLiteral("sha1")).toString();
    download.size = static_cast<qint64>(obj.value(QStringLiteral("size")).toDouble());
    return download;
}

static mc::Library parseLibrary(const QJsonObject& obj) {
    mc::Library lib;
    lib.name = obj.value(QStringLiteral("name")).toString();
    lib.url = obj.value(QStringLiteral("url")).toString();
    lib.rules = parseRules(obj.value(QStringLiteral("rules")));

    const QJsonValue natives = obj.value(QStringLiteral("natives"));
    if (natives.isArray()) {
        for (const QJsonValue& entry : natives.toArray())
            if (entry.isString()) lib.natives.append(entry.toString());
    } else if (natives.isObject()) {
        const QJsonObject nativesObj = natives.toObject();
        for (auto it = nativesObj.begin(); it != nativesObj.end(); ++it)
            if (it.value().isString()) lib.natives.append(it.value().toString());
    }
    lib.isNative = !lib.natives.isEmpty();

    const QJsonObject extract = obj.value(QStringLiteral("extract")).toObject();
    for (const QJsonValue& entry : extract.value(QStringLiteral("exclude")).toArray())
        if (entry.isString()) lib.extractExclude.insert(entry.toString(), true);

    const QJsonObject downloads = obj.value(QStringLiteral("downloads")).toObject();
    const QJsonValue artifact = downloads.value(QStringLiteral("artifact"));
    if (artifact.isObject()) lib.artifact = parseDownload(artifact.toObject());

    const QJsonObject classifiers = downloads.value(QStringLiteral("classifiers")).toObject();
    for (auto it = classifiers.begin(); it != classifiers.end(); ++it)
        if (it.value().isObject()) lib.classifiers.append(parseDownload(it.value().toObject()));

    return lib;
}

static void appendArgument(const QJsonValue& val, QVector<mc::ArgumentValue>* out) {
    if (val.isString()) {
        mc::ArgumentValue arg;
        arg.value = val.toString();
        out->append(arg);
        return;
    }
    if (!val.isObject()) return;

    const QJsonObject obj = val.toObject();
    const QJsonValue value = obj.value(QStringLiteral("value"));
    QStringList values;
    if (value.isString()) {
        values.append(value.toString());
    } else if (value.isArray()) {
        for (const QJsonValue& entry : value.toArray())
            if (entry.isString()) values.append(entry.toString());
    }

    for (const QString& text : values) {
        mc::ArgumentValue arg;
        arg.value = text;
        arg.rules = parseRules(obj.value(QStringLiteral("rules")));
        out->append(arg);
    }
}

static void parseArguments(const QJsonValue& val, QVector<mc::ArgumentValue>* out) {
    if (!val.isArray()) return;
    for (const QJsonValue& entry : val.toArray())
        appendArgument(entry, out);
}

static QString libraryCoordinate(const mc::Library& lib) {
    const QStringList parts = lib.name.split(QLatin1Char(':'));
    if (parts.size() >= 2) return parts.at(0) + QLatin1Char(':') + parts.at(1);
    return lib.name;
}

static QJsonArray serializeRules(const QVector<mc::LibraryRule>& rules) {
    QJsonArray arr;
    for (const mc::LibraryRule& rule : rules) {
        QJsonObject obj;
        obj.insert(QStringLiteral("action"), rule.action);
        if (!rule.osName.isEmpty()) {
            QJsonObject os;
            os.insert(QStringLiteral("name"), rule.osName);
            obj.insert(QStringLiteral("os"), os);
        }
        arr.append(obj);
    }
    return arr;
}

static bool hasDownload(const mc::LibraryDownloads& download) {
    return !download.path.isEmpty() || !download.url.isEmpty() || !download.sha1.isEmpty() ||
           download.size > 0;
}

static QJsonObject serializeDownload(const mc::LibraryDownloads& download) {
    QJsonObject obj;
    if (!download.path.isEmpty()) obj.insert(QStringLiteral("path"), download.path);
    if (!download.url.isEmpty()) obj.insert(QStringLiteral("url"), download.url);
    if (!download.sha1.isEmpty()) obj.insert(QStringLiteral("sha1"), download.sha1);
    if (download.size > 0) obj.insert(QStringLiteral("size"), download.size);
    return obj;
}

static QJsonObject serializeLibrary(const mc::Library& lib) {
    QJsonObject obj;
    obj.insert(QStringLiteral("name"), lib.name);
    if (!lib.url.isEmpty()) obj.insert(QStringLiteral("url"), lib.url);

    if (!lib.natives.isEmpty()) {
        QJsonArray natives;
        for (const QString& native : lib.natives) natives.append(native);
        obj.insert(QStringLiteral("natives"), natives);
    }

    if (!lib.rules.isEmpty()) obj.insert(QStringLiteral("rules"), serializeRules(lib.rules));

    QJsonObject downloads;
    if (hasDownload(lib.artifact))
        downloads.insert(QStringLiteral("artifact"), serializeDownload(lib.artifact));
    if (!lib.classifiers.isEmpty()) {
        QJsonObject classifiers;
        for (const mc::LibraryDownloads& download : lib.classifiers) {
            QString key = QFileInfo(download.path).fileName();
            if (key.endsWith(QLatin1String(".jar"))) key.chop(4);
            if (key.isEmpty()) key = QString::number(classifiers.size());
            classifiers.insert(key, serializeDownload(download));
        }
        downloads.insert(QStringLiteral("classifiers"), classifiers);
    }
    if (!downloads.isEmpty()) obj.insert(QStringLiteral("downloads"), downloads);

    if (!lib.extractExclude.isEmpty()) {
        QJsonArray exclude;
        for (auto it = lib.extractExclude.begin(); it != lib.extractExclude.end(); ++it)
            exclude.append(it.key());
        QJsonObject extract;
        extract.insert(QStringLiteral("exclude"), exclude);
        obj.insert(QStringLiteral("extract"), extract);
    }
    return obj;
}

static QJsonArray serializeArguments(const QVector<mc::ArgumentValue>& args) {
    QJsonArray arr;
    for (const mc::ArgumentValue& arg : args) {
        if (arg.rules.isEmpty()) {
            arr.append(arg.value);
        } else {
            QJsonObject obj;
            obj.insert(QStringLiteral("value"), arg.value);
            obj.insert(QStringLiteral("rules"), serializeRules(arg.rules));
            arr.append(obj);
        }
    }
    return arr;
}

static QJsonObject serializeVersion(const mc::VersionJson& version) {
    QJsonObject obj;
    if (!version.id.isEmpty()) obj.insert(QStringLiteral("id"), version.id);
    if (!version.mainClass.isEmpty()) obj.insert(QStringLiteral("mainClass"), version.mainClass);
    if (!version.type.isEmpty()) obj.insert(QStringLiteral("type"), version.type);
    if (!version.assets.isEmpty()) obj.insert(QStringLiteral("assets"), version.assets);
    if (!version.minecraftArguments.isEmpty())
        obj.insert(QStringLiteral("minecraftArguments"), version.minecraftArguments);
    if (!version.processArguments.isEmpty())
        obj.insert(QStringLiteral("processArguments"), version.processArguments);
    if (!version.releaseTime.isEmpty())
        obj.insert(QStringLiteral("releaseTime"), version.releaseTime);
    if (!version.time.isEmpty()) obj.insert(QStringLiteral("time"), version.time);
    if (version.minimumLauncherVersion > 0)
        obj.insert(QStringLiteral("minimumLauncherVersion"), version.minimumLauncherVersion);

    if (!version.compatibleJavaMajors.isEmpty()) {
        QJsonArray majors;
        for (const QString& major : version.compatibleJavaMajors) majors.append(major);
        obj.insert(QStringLiteral("compatibleJavaMajors"), majors);
    }

    if (!version.libraries.isEmpty()) {
        QJsonArray libraries;
        for (const mc::Library& lib : version.libraries)
            libraries.append(serializeLibrary(lib));
        obj.insert(QStringLiteral("libraries"), libraries);
    }

    QJsonObject args;
    if (!version.gameArgs.isEmpty()) args.insert(QStringLiteral("game"), serializeArguments(version.gameArgs));
    if (!version.jvmArgs.isEmpty()) args.insert(QStringLiteral("jvm"), serializeArguments(version.jvmArgs));
    if (!args.isEmpty()) obj.insert(QStringLiteral("arguments"), args);

    if (!version.assetIndex.isEmpty()) obj.insert(QStringLiteral("assetIndex"), version.assetIndex);
    if (!version.downloads.isEmpty()) obj.insert(QStringLiteral("downloads"), version.downloads);
    if (!version.javaVersion.isEmpty()) obj.insert(QStringLiteral("javaVersion"), version.javaVersion);
    return obj;
}

static mc::VersionJson mergeVersions(const mc::VersionJson& parent, const mc::VersionJson& child) {
    mc::VersionJson merged = parent;
    if (!child.id.isEmpty()) merged.id = child.id;
    if (!child.mainClass.isEmpty()) merged.mainClass = child.mainClass;
    if (!child.type.isEmpty()) merged.type = child.type;
    if (!child.assets.isEmpty()) merged.assets = child.assets;
    if (!child.minecraftArguments.isEmpty()) merged.minecraftArguments = child.minecraftArguments;
    if (!child.processArguments.isEmpty()) merged.processArguments = child.processArguments;
    if (!child.releaseTime.isEmpty()) merged.releaseTime = child.releaseTime;
    if (!child.time.isEmpty()) merged.time = child.time;
    if (child.minimumLauncherVersion > 0)
        merged.minimumLauncherVersion = child.minimumLauncherVersion;
    if (!child.compatibleJavaMajors.isEmpty())
        merged.compatibleJavaMajors = child.compatibleJavaMajors;
    if (!child.assetIndex.isEmpty()) merged.assetIndex = child.assetIndex;
    if (!child.downloads.isEmpty()) merged.downloads = child.downloads;
    if (!child.javaVersion.isEmpty()) merged.javaVersion = child.javaVersion;
    merged.inheritsFrom.clear();

    QSet<QString> childKeys;
    for (const mc::Library& lib : child.libraries)
        childKeys.insert(libraryCoordinate(lib));
    merged.libraries = child.libraries;
    for (const mc::Library& lib : parent.libraries) {
        if (!childKeys.contains(libraryCoordinate(lib))) merged.libraries.append(lib);
    }

    merged.gameArgs = parent.gameArgs;
    merged.gameArgs += child.gameArgs;
    merged.jvmArgs = parent.jvmArgs;
    merged.jvmArgs += child.jvmArgs;
    return merged;
}

namespace mc {

std::optional<VersionJson> parseVersionJson(const QByteArray& data) {
    const auto obj = json::parseObject(data);
    if (!obj) return std::nullopt;

    VersionJson version;
    version.id = obj->value(QStringLiteral("id")).toString();
    version.mainClass = obj->value(QStringLiteral("mainClass")).toString();
    version.type = obj->value(QStringLiteral("type")).toString();
    version.assets = obj->value(QStringLiteral("assets")).toString();
    version.inheritsFrom = obj->value(QStringLiteral("inheritsFrom")).toString();
    version.minecraftArguments = obj->value(QStringLiteral("minecraftArguments")).toString();
    version.processArguments = obj->value(QStringLiteral("processArguments")).toString();
    version.releaseTime = obj->value(QStringLiteral("releaseTime")).toString();
    version.time = obj->value(QStringLiteral("time")).toString();
    version.minimumLauncherVersion = obj->value(QStringLiteral("minimumLauncherVersion")).toInt();
    version.assetIndex = obj->value(QStringLiteral("assetIndex")).toObject();
    version.downloads = obj->value(QStringLiteral("downloads")).toObject();
    version.javaVersion = obj->value(QStringLiteral("javaVersion")).toObject();

    for (const QJsonValue& entry : obj->value(QStringLiteral("compatibleJavaMajors")).toArray()) {
        if (entry.isString()) {
            version.compatibleJavaMajors.append(entry.toString());
        } else if (entry.isDouble()) {
            version.compatibleJavaMajors.append(QString::number(entry.toInt()));
        }
    }

    for (const QJsonValue& entry : obj->value(QStringLiteral("libraries")).toArray())
        if (entry.isObject()) version.libraries.append(parseLibrary(entry.toObject()));

    const QJsonObject args = obj->value(QStringLiteral("arguments")).toObject();
    parseArguments(args.value(QStringLiteral("game")), &version.gameArgs);
    parseArguments(args.value(QStringLiteral("jvm")), &version.jvmArgs);
    return version;
}

std::optional<VersionJson> loadVersionJson(const QString& path) {
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) return std::nullopt;
    return parseVersionJson(file.readAll());
}

std::optional<QString> resolveInheritance(const VersionJson& child, const QString& versionsDir) {
    if (child.inheritsFrom.isEmpty()) return std::nullopt;

    const QString parentPath = QDir(versionsDir).filePath(
        child.inheritsFrom + QStringLiteral("/") + child.inheritsFrom + QStringLiteral(".json"));
    const auto parentJson = loadVersionJson(parentPath);
    if (!parentJson) return std::nullopt;

    VersionJson parent = *parentJson;
    if (!parent.inheritsFrom.isEmpty()) {
        if (const auto inherited = resolveInheritance(parent, versionsDir)) {
            if (const auto parsed = parseVersionJson(inherited->toUtf8())) parent = *parsed;
        }
    }

    const VersionJson merged = mergeVersions(parent, child);
    return QString::fromUtf8(
        QJsonDocument(serializeVersion(merged)).toJson(QJsonDocument::Compact));
}

bool ruleAllows(const QVector<LibraryRule>& rules, const QString& osName) {
    if (rules.isEmpty()) return true;

    bool result = false;
    for (const LibraryRule& rule : rules) {
        if (!rule.osName.isEmpty() && rule.osName != osName) continue;
        result = rule.action == QLatin1String("allow");
    }
    return result;
}

QString libraryPath(const Library& lib, const QString& librariesRoot) {
    if (!lib.artifact.path.isEmpty())
        return QDir(librariesRoot).filePath(lib.artifact.path);

    QString base = lib.name;
    QString ext = QStringLiteral("jar");
    const int at = base.indexOf(QLatin1Char('@'));
    if (at >= 0) {
        ext = base.mid(at + 1);
        base = base.left(at);
    }

    const QStringList parts = base.split(QLatin1Char(':'));
    if (parts.size() < 3) return {};

    QString group = parts.at(0);
    group.replace(QLatin1Char('.'), QLatin1Char('/'));
    const QString artifact = parts.at(1);
    const QString version = parts.at(2);

    QString file = artifact + QLatin1Char('-') + version;
    if (parts.size() > 3) file += QLatin1Char('-') + parts.at(3);
    file += QLatin1Char('.') + ext;

    const QString rel = group + QLatin1Char('/') + artifact + QLatin1Char('/') + version +
                        QLatin1Char('/') + file;
    return QDir(librariesRoot).filePath(rel);
}

QString nativeClassifier(const Library& lib, const QString& osArch) {
    if (lib.natives.isEmpty()) return {};

    QString os = osArch;
    QString arch;
    const int dash = osArch.indexOf(QLatin1Char('-'));
    if (dash >= 0) {
        os = osArch.left(dash);
        arch = osArch.mid(dash + 1);
    }
    if (arch == QLatin1String("x86_64") || arch == QLatin1String("x64"))
        arch = QStringLiteral("amd64");
    else if (arch == QLatin1String("aarch64"))
        arch = QStringLiteral("arm64");

    QString bits = QStringLiteral("64");
    if (arch == QLatin1String("x86") || arch == QLatin1String("i386") ||
        arch == QLatin1String("i686") || arch == QLatin1String("32"))
        bits = QStringLiteral("32");

    QString fallback;
    for (const QString& native : lib.natives) {
        if (!native.contains(os)) continue;
        QString classifier = native;
        if (classifier.contains(QLatin1String("${arch}")))
            classifier.replace(QLatin1String("${arch}"), bits);
        if (arch.isEmpty() || classifier.contains(os + QLatin1Char('-') + arch))
            return classifier;
        if (fallback.isEmpty()) fallback = classifier;
    }
    return fallback;
}

} // namespace mc
