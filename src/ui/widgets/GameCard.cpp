#include "ui/widgets/GameCard.h"
#include "ui/Theme.h"
#include "ui/Fonts.h"
#include <QPainter>
#include <QPainterPath>
#include <QFont>
#include <QFontMetrics>
#include <QConicalGradient>
#include <QLinearGradient>
#include <QMouseEvent>
#include <cmath>

using namespace theme::C;

static float easeOutCubic(float t) { return 1.0f - std::pow(1.0f - t, 3.0f); }
static float easeInOutCubic(float t) { return t < 0.5f ? 4*t*t*t : 1-std::pow(-2*t+2,3)/2; }

static QColor withAlpha(const QColor& c, int a) {
    return QColor(c.red(), c.green(), c.blue(), a);
}

GameCard::GameCard(const Game& game, int cardW, int cardH, QWidget* parent)
    : QWidget(parent), m_game(game), m_cardW(cardW), m_cardH(cardH)
{
    setFixedSize(cardW, cardH);
    setCursor(Qt::PointingHandCursor);
    setAttribute(Qt::WA_Hover, true);

    connect(&m_hoverAnim, &QTimer::timeout, this, &GameCard::hoverTick);
    m_hoverAnim.setInterval(16);
}

void GameCard::setArt(const QPixmap& pix) { m_art = pix; update(); }
void GameCard::setFocused(bool focused) { m_focused = focused; startHoverAnim(); update(); }

float GameCard::hoverLevel() const { return easeOutCubic(m_hprog); }

void GameCard::startHoverAnim() {
    if (!m_hoverAnim.isActive()) m_hoverAnim.start();
}

void GameCard::hoverTick() {
    float dt = m_hoverAnim.interval() / 1000.0f;
    bool wanted = m_focused || underMouse();
    float target = wanted ? 1.0f : 0.0f;

    if (m_hprog < target) m_hprog = std::min(target, m_hprog + dt / 0.18f);
    else if (m_hprog > target) m_hprog = std::max(target, m_hprog - dt / 0.18f);

    if (wanted) m_hoverClock += dt; else m_hoverClock = 0.0f;

    update();
    if (m_hprog == target && !wanted) m_hoverAnim.stop();
}

void GameCard::paintEvent(QPaintEvent*) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    p.setRenderHint(QPainter::SmoothPixmapTransform);

    int w = width(), h = height();
    QRectF rect(0.5, 0.5, w - 1, h - 1);
    QPainterPath path;
    path.addRoundedRect(rect, RADIUS, RADIUS);
    p.setClipPath(path);

    p.fillRect(rect, QColor(COLOR_3));

    float t = hoverLevel();
    float zoom = 1.0f + 0.045f * t;
    QRectF drawRect(rect.center().x() - rect.width()*zoom/2, rect.center().y() - rect.height()*zoom/2,
                    rect.width()*zoom, rect.height()*zoom);

    if (!m_art.isNull()) {
        QPixmap scaled = m_art.scaled(drawRect.size().toSize(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        int ox = (scaled.width() - (int)drawRect.width()) / 2;
        int oy = (scaled.height() - (int)drawRect.height()) / 2;
        p.drawPixmap(drawRect, scaled, QRectF(ox, oy, drawRect.width(), drawRect.height()));
    } else {
        paintPlaceholder(p, w, h, drawRect);
    }

    bool overlayVisible = m_focused || underMouse();
    if (overlayVisible) paintOverlay(p, w, h);

    p.setClipping(false);
    if (t > 0) paintHoverBorder(p, rect, t);
    if (m_focused) {
        p.setPen(QPen(QColor(ACCENT_1), 2));
        p.drawRoundedRect(rect, RADIUS, RADIUS);
    }
}

void GameCard::paintPlaceholder(QPainter& p, int w, int h, const QRectF& target) {
    Q_UNUSED(target);
    p.fillRect(QRectF(0, 0, w, h), QColor(COLOR_3));

    QFont font(fonts::UI_FONT, std::max(9, w / 21), QFont::Medium);
    p.setFont(font);
    QFontMetrics fm(font);
    QString label = m_game.name();
    while (fm.horizontalAdvance(label) > w - 24 && label.length() > 10)
        label = label.left(label.length() - 3) + QStringLiteral("\u2026");
    p.setPen(QColor(TEXT_DIM));
    p.drawText(QRectF(12, h - 40, w - 24, 24), Qt::AlignCenter, label);
}

void GameCard::paintOverlay(QPainter& p, int w, int h) {
    int overlayH = std::max(44, h / 5);
    QLinearGradient grad(0, h - overlayH, 0, h);
    grad.setColorAt(0, QColor(0, 0, 0, 0));
    grad.setColorAt(1, QColor(0, 0, 0, 205));
    p.fillRect(QRectF(0, h - overlayH, w, overlayH), grad);

    QFont font(fonts::UI_FONT, std::max(10, w / 19), QFont::DemiBold);
    p.setFont(font);
    QFontMetrics fm(font);
    QString label = m_game.name();
    while (fm.horizontalAdvance(label) > w - 20 && label.length() > 10)
        label = label.left(label.length() - 3) + QStringLiteral("\u2026");
    p.setPen(QColor(TEXT));
    p.drawText(QRectF(10, h - overlayH + 8, w - 20, overlayH - 12),
               Qt::AlignLeft | Qt::AlignVCenter, label);
}

void GameCard::paintHoverBorder(QPainter& p, const QRectF& rect, float t) {
    QPen pen(withAlpha(QColor(ACCENT_1), (int)(55 * t)));
    pen.setWidth(2);
    p.setPen(pen);
    p.setBrush(Qt::NoBrush);
    p.drawRoundedRect(rect, RADIUS, RADIUS);

    float angle = std::fmod(m_hoverClock, 1.0f) / 1.0f * 360.0f;
    QConicalGradient cg(rect.center(), angle);
    cg.setColorAt(0.0, withAlpha(QColor(ACCENT_1), (int)(235 * t)));
    cg.setColorAt(0.14, withAlpha(QColor(ACCENT_1), 0));
    cg.setColorAt(1.0, withAlpha(QColor(ACCENT_1), 0));
    p.setPen(QPen(QBrush(cg), 3.0f));
    p.drawRoundedRect(rect, RADIUS, RADIUS);
}

void GameCard::enterEvent(QEnterEvent*) { startHoverAnim(); update(); }
void GameCard::leaveEvent(QEvent*) { startHoverAnim(); update(); }

void GameCard::mousePressEvent(QMouseEvent* e) {
    if (e->button() == Qt::LeftButton) emit clicked(m_game);
}
void GameCard::mouseDoubleClickEvent(QMouseEvent* e) {
    if (e->button() == Qt::LeftButton) emit activated(m_game);
}
