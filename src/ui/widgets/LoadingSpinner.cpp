#include "ui/widgets/LoadingSpinner.h"
#include "ui/Theme.h"
#include <QPainter>
#include <QColor>

LoadingSpinner::LoadingSpinner(QWidget* parent)
    : QWidget(parent)
{
    setFixedSize(48, 48);
    connect(&m_timer, &QTimer::timeout, this, &LoadingSpinner::rotate);
}

void LoadingSpinner::rotate() {
    m_angle = (m_angle + 30) % 360;
    update();
}

void LoadingSpinner::paintEvent(QPaintEvent*) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    p.translate(24, 24);
    p.rotate(m_angle);
    QColor accent(theme::C::ACCENT_1);
    for (int i = 0; i < 8; ++i) {
        int alpha = 255 - i * 32;
        p.setBrush(QColor(accent.red(), accent.green(), accent.blue(), alpha));
        p.setPen(Qt::NoPen);
        p.drawEllipse(-3, -16, 6, 6);
        p.rotate(45);
    }
}

void LoadingSpinner::showEvent(QShowEvent* e) { QWidget::showEvent(e); m_timer.start(50); }
void LoadingSpinner::hideEvent(QHideEvent* e) { QWidget::hideEvent(e); m_timer.stop(); }
