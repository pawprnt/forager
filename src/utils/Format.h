#pragma once

#include <QString>

namespace format {
    inline QString size(qint64 bytes) {
        const char* units[] = {"B", "KB", "MB", "GB", "TB"};
        double num = static_cast<double>(bytes);
        for (int i = 0; i < 5; ++i) {
            if (num < 1024)
                return QStringLiteral("%1 %2").arg(num, 0, 'f', 1).arg(units[i]);
            num /= 1024;
        }
        return QStringLiteral("%1 PB").arg(num, 0, 'f', 1);
    }
} // namespace format
