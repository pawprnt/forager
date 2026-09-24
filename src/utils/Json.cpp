#include "utils/Json.h"
#include "utils/Filesystem.h"
#include <QFile>
#include <QJsonDocument>

std::optional<QJsonObject> json::readObject(const QString& path) {
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly)) return std::nullopt;

    QJsonParseError err{};
    QJsonDocument doc = QJsonDocument::fromJson(f.readAll(), &err);
    if (err.error != QJsonParseError::NoError || !doc.isObject()) return std::nullopt;
    return doc.object();
}

bool json::writeObject(const QString& path, const QJsonObject& obj, bool indented) {
    if (!fs::ensureParentDir(path)) return false;

    QFile f(path);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate)) return false;

    const QJsonDocument doc(obj);
    f.write(indented ? doc.toJson(QJsonDocument::Indented)
                     : doc.toJson(QJsonDocument::Compact));
    return true;
}

std::optional<QJsonObject> json::parseObject(const QByteArray& data) {
    QJsonParseError err{};
    QJsonDocument doc = QJsonDocument::fromJson(data, &err);
    if (err.error != QJsonParseError::NoError || !doc.isObject()) return std::nullopt;
    return doc.object();
}
