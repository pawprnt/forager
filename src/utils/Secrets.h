#pragma once

#include <QString>

namespace secrets {
    bool store(const QString& key, const QString& value);
    QString lookup(const QString& key);
    bool clear(const QString& key);
} // namespace secrets
