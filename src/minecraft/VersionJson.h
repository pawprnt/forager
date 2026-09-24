#pragma once

#include <QString>
#include <QStringList>
#include <QJsonObject>
#include <QJsonArray>
#include <QVector>
#include <optional>

namespace mc {

struct LibraryRule {
    QString action;
    QString osName;
};

struct LibraryDownloads {
    QString path;
    QString url;
    QString sha1;
    qint64 size = 0;
};

struct Library {
    QString name;
    QString url;
    QStringList natives;
    QVector<LibraryRule> rules;
    LibraryDownloads artifact;
    QVector<LibraryDownloads> classifiers;
    QJsonObject extractExclude;
    bool isNative = false;
};

struct ArgumentValue {
    QString value;
    QVector<LibraryRule> rules;
};

struct VersionJson {
    QString id;
    QString mainClass;
    QString type;
    QString assets;
    QString inheritsFrom;
    QString minecraftArguments;
    QString processArguments;
    QVector<Library> libraries;
    QVector<ArgumentValue> gameArgs;
    QVector<ArgumentValue> jvmArgs;
    QJsonObject assetIndex;
    QJsonObject downloads;
    QJsonObject javaVersion;
    QStringList compatibleJavaMajors;
    QString releaseTime;
    QString time;
    int minimumLauncherVersion = 0;
};

std::optional<VersionJson> parseVersionJson(const QByteArray& data);
std::optional<VersionJson> loadVersionJson(const QString& path);

std::optional<QString> resolveInheritance(const VersionJson& child, const QString& versionsDir);

bool ruleAllows(const QVector<LibraryRule>& rules, const QString& osName);
QString libraryPath(const Library& lib, const QString& librariesRoot);
QString nativeClassifier(const Library& lib, const QString& osArch);

} // namespace mc
