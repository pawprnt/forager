#pragma once
#include <QByteArray>
#include <QJsonObject>
#include <QString>
#include <optional>

namespace json {
    std::optional<QJsonObject> readObject(const QString& path);
    bool writeObject(const QString& path, const QJsonObject& obj, bool indented = true);
    std::optional<QJsonObject> parseObject(const QByteArray& data);
} // namespace json
