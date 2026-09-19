#include "ui/widgets/Banner.h"
#include "ui/Theme.h"
#include <QPainter>
#include <QPainterPath>
#include <QImage>
#include <QQueue>

using namespace theme::C;

Banner::Banner(QWidget* parent)
    : QWidget(parent)
{
    setMinimumHeight(BANNER_H);
    setMaximumHeight(BANNER_H);
    m_backdropCacheKey = {-1, 0, 0};
    m_crispCacheKey = {-1, 0, 0};
}

void Banner::setSource(const QPixmap& pix, bool fit) {
    m_source = pix;
    m_fit = fit;
    m_backdropCacheKey = {-1, 0, 0};
    m_crispCacheKey = {-1, 0, 0};
    update();
}

void Banner::setOverlay(QWidget* overlay) {
    m_overlay = overlay;
    if (overlay) overlay->setParent(this);
}

void Banner::resizeEvent(QResizeEvent* event) {
    QWidget::resizeEvent(event);
    if (m_overlay) m_overlay->setGeometry(rect());
}

void Banner::paintEvent(QPaintEvent*) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    p.setRenderHint(QPainter::SmoothPixmapTransform);
    p.setBrush(QColor(COLOR_1));
    p.setPen(Qt::NoPen);
    p.drawRoundedRect(rect(), RADIUS, RADIUS);
    if (m_source.isNull()) return;

    p.save();
    p.setClipPath(clipPath());

    int w = width(), h = height();
    int sw = m_source.width(), sh = m_source.height();
    double scale = qMin(static_cast<double>(w) / sw, static_cast<double>(h) / sh);
    double dispW = sw * scale, dispH = sh * scale;

    p.drawPixmap(0, 0, backdrop(m_source));

    if (m_fit) {
        p.drawPixmap(QRectF((w - dispW) / 2, (h - dispH) / 2, dispW, dispH),
                     m_source, QRectF(0, 0, sw, sh));
    } else {
        p.drawPixmap(QRectF((w - dispW) / 2, (h - dispH) / 2, dispW, dispH),
                     crisp(m_source), QRectF(0, 0, dispW, dispH));
    }

    p.restore();
}

static QPixmap gaussianBlur(const QPixmap& input, double radius) {
    if (input.isNull()) return input;
    QImage src = input.toImage().convertToFormat(QImage::Format_RGBA8888);
    int w = src.width(), h = src.height();
    int r = static_cast<int>(radius);
    int kernelSize = r * 2 + 1;

    std::vector<double> kernel(kernelSize);
    double sigma = radius / 3.0;
    double sum = 0;
    for (int i = 0; i < kernelSize; ++i) {
        double x = i - r;
        kernel[i] = std::exp(-(x * x) / (2 * sigma * sigma));
        sum += kernel[i];
    }
    for (auto& k : kernel) k /= sum;

    QImage temp(w, h, QImage::Format_RGBA8888);
    QImage dst(w, h, QImage::Format_RGBA8888);

    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            double r0 = 0, g = 0, b = 0, a = 0;
            for (int k = -r; k <= r; ++k) {
                int sx = qBound(0, x + k, w - 1);
                QRgb pixel = src.pixel(sx, y);
                double kw = kernel[k + r];
                r0 += qRed(pixel) * kw;
                g += qGreen(pixel) * kw;
                b += qBlue(pixel) * kw;
                a += qAlpha(pixel) * kw;
            }
            temp.setPixel(x, y, qRgba(static_cast<int>(r0), static_cast<int>(g),
                                      static_cast<int>(b), static_cast<int>(a)));
        }
    }
    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            double r0 = 0, g = 0, b = 0, a = 0;
            for (int k = -r; k <= r; ++k) {
                int sy = qBound(0, y + k, h - 1);
                QRgb pixel = temp.pixel(x, sy);
                double kw = kernel[k + r];
                r0 += qRed(pixel) * kw;
                g += qGreen(pixel) * kw;
                b += qBlue(pixel) * kw;
                a += qAlpha(pixel) * kw;
            }
            dst.setPixel(x, y, qRgba(static_cast<int>(r0), static_cast<int>(g),
                                     static_cast<int>(b), static_cast<int>(a)));
        }
    }
    return QPixmap::fromImage(dst);
}

static QPixmap featherEdges(const QPixmap& input, int fade) {
    if (fade <= 0 || input.isNull()) return input;
    QImage img = input.toImage().convertToFormat(QImage::Format_RGBA8888);
    int w = img.width(), h = img.height();

    std::vector<int> hMask(w), vMask(h);
    for (int x = 0; x < w; ++x) {
        int d = qMin(x, w - 1 - x);
        hMask[x] = qMin(255, static_cast<int>(255.0 * d / fade));
    }
    for (int y = 0; y < h; ++y) {
        int d = qMin(y, h - 1 - y);
        vMask[y] = qMin(255, static_cast<int>(255.0 * d / fade));
    }

    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            int alpha = (hMask[x] * vMask[y]) / 255;
            QRgb pixel = img.pixel(x, y);
            img.setPixel(x, y, qRgba(qRed(pixel), qGreen(pixel), qBlue(pixel), alpha));
        }
    }
    return QPixmap::fromImage(img);
}

QPixmap Banner::backdrop(const QPixmap& pix) {
    int w = width(), h = height();
    if (w <= 0 || h <= 0 || pix.isNull()) return pix;

    auto key = std::make_tuple(pix.cacheKey(), w, h);
    if (m_backdropCacheKey == key) return m_backdropCache;

    double k = qMax(static_cast<double>(w) / pix.width(), static_cast<double>(h) / pix.height());
    QPixmap cover = pix.scaled(static_cast<int>(pix.width() * k), static_cast<int>(pix.height() * k),
                               Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    int x = (cover.width() - w) / 2;
    int y = (cover.height() - h) / 2;
    cover = cover.copy(x, y, w, h);
    QPixmap blurred = gaussianBlur(cover, BLUR_RADIUS);

    m_backdropCacheKey = key;
    m_backdropCache = blurred;
    return blurred;
}

QPixmap Banner::crisp(const QPixmap& pix) {
    int w = width(), h = height();
    auto key = std::make_tuple(pix.cacheKey(), w, h);
    if (m_crispCacheKey == key) return m_crispCache;

    QPixmap scaled = pix.scaled(w, h, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    bool hFeather = scaled.width() < w;
    bool vFeather = scaled.height() < h;

    QPixmap feathered = scaled;
    if (hFeather || vFeather) {
        feathered = featherEdges(scaled, FEATHER);
    }

    m_crispCacheKey = key;
    m_crispCache = feathered;
    return feathered;
}

QPainterPath Banner::clipPath() const {
    QPainterPath path;
    path.addRoundedRect(rect(), RADIUS, RADIUS);
    return path;
}
