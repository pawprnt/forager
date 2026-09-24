#pragma once
#include <QPixmap>
#include <QByteArray>

namespace pixmap {
    QPixmap scaled(const QPixmap& src, int w, int h);
    QPixmap scaleCrop(const QPixmap& src, int w, int h);
    QByteArray toJpeg(const QPixmap& pix, int quality = 85);
} // namespace pixmap
