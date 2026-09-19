#include "artwork/Placeholder.h"
#include "ui/Theme.h"
#include "ui/Fonts.h"
#include <QPainter>
#include <QLinearGradient>
#include <QFont>

using namespace theme::C;

QPixmap placeholder::placeholderGrid(const QString& name, int w, int h) {
    QPixmap pix(w, h);
    pix.fill(QColor(COLOR_3));
    QPainter p(&pix);
    p.setRenderHint(QPainter::Antialiasing);

    QLinearGradient grad(0, 0, 0, h);
    grad.setColorAt(0, QColor(ACCENT_1));
    grad.setColorAt(1, QColor(BG));
    p.setOpacity(0.08);
    p.fillRect(0, 0, w, h, grad);
    p.setOpacity(1.0);

    QFont font(fonts::UI_FONT, std::max(9, w / 21), QFont::Medium);
    p.setFont(font);
    p.setPen(QColor(TEXT_DIM));
    QString label = name;
    QFontMetrics fm(font);
    while (fm.horizontalAdvance(label) > w - 24 && label.length() > 10)
        label = label.left(label.length() - 3) + QStringLiteral("\u2026");
    p.drawText(QRectF(12, h - 40, w - 24, 24), Qt::AlignCenter, label);
    return pix;
}

QPixmap placeholder::placeholderCard(const QString& name, int w, int h) {
    return placeholderGrid(name, w, h);
}
