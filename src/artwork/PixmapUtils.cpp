#include "artwork/PixmapUtils.h"

QPixmap pixmap::scaled(const QPixmap& src, int w, int h) {
    return src.scaled(w, h, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
}

QPixmap pixmap::scaleCrop(const QPixmap& src, int w, int h) {
    if (src.isNull()) return src;
    QPixmap expanded = src.scaled(w, h, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
    int ox = (expanded.width() - w) / 2;
    int oy = (expanded.height() - h) / 2;
    return expanded.copy(ox, oy, w, h);
}
