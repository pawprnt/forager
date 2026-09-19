#pragma once
#include <QPixmap>

namespace pixmap {
    QPixmap scaled(const QPixmap& src, int w, int h);
    QPixmap scaleCrop(const QPixmap& src, int w, int h);
} // namespace pixmap
