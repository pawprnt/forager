#pragma once
#include <QPixmap>
#include <QString>

namespace placeholder {
    QPixmap placeholderGrid(const QString& name, int w, int h);
    QPixmap placeholderCard(const QString& name, int w, int h);
} // namespace placeholder
