#include "minecraft/Instance.h"

#include "utils/Json.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>

namespace mc {

std::optional<QString> findVersionJson(const QString& versionsDir, const QString& versionId) {
    if (versionId.isEmpty()) return std::nullopt;
    const QString rel = versionId + QStringLiteral("/") + versionId + QStringLiteral(".json");
    const QString path = QDir(versionsDir).filePath(rel);
    if (QFileInfo::exists(path)) return path;
    return std::nullopt;
}

std::optional<QString> detectVersionId(const QString& instanceDir) {
    const QDir dir(instanceDir);

    const auto pack = json::readObject(dir.filePath(QStringLiteral("mmc-pack.json")));
    if (pack) {
        for (const QJsonValue& entry : pack->value(QStringLiteral("components")).toArray()) {
            const QJsonObject component = entry.toObject();
            if (component.value(QStringLiteral("uid")).toString() !=
                QLatin1String("net.minecraft"))
                continue;
            const QString version = component.value(QStringLiteral("version")).toString();
            if (!version.isEmpty()) return version;
        }
    }

    const QStringList names =
        dir.entryList({QStringLiteral("*.json")}, QDir::Files, QDir::Name);
    for (const QString& name : names) {
        if (name == QLatin1String("mmc-pack.json")) continue;
        const auto obj = json::readObject(dir.filePath(name));
        if (!obj) continue;
        QString id = obj->value(QStringLiteral("id")).toString();
        if (id.isEmpty()) id = obj->value(QStringLiteral("version")).toString();
        if (!id.isEmpty()) return id;
    }

    QFile cfg(dir.filePath(QStringLiteral("instance.cfg")));
    if (cfg.open(QIODevice::ReadOnly | QIODevice::Text)) {
        while (!cfg.atEnd()) {
            const QString line = QString::fromUtf8(cfg.readLine()).trimmed();
            if (!line.startsWith(QStringLiteral("Version="))) continue;
            const QString version = line.mid(QStringLiteral("Version=").size());
            if (!version.isEmpty()) return version;
        }
    }

    return std::nullopt;
}

QString resolveGameRoot(const QString& instanceDir) {
    const QDir dir(instanceDir);
    const bool plain = dir.exists(QStringLiteral("minecraft"));
    const bool dotted = dir.exists(QStringLiteral(".minecraft"));
    if (plain && !dotted) return dir.filePath(QStringLiteral("minecraft"));
    if (dotted) return dir.filePath(QStringLiteral(".minecraft"));
    return instanceDir;
}

QString resolveNativesDir(const QString& instanceDir) {
    return QDir(instanceDir).filePath(QStringLiteral("natives"));
}

} // namespace mc
