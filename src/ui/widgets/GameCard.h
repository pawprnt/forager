#pragma once

#include <QWidget>
#include <QTimer>
#include <QPixmap>
#include "core/Game.h"

class GameCard : public QWidget {
    Q_OBJECT

public:
    explicit GameCard(const Game& game, int cardW = 165, int cardH = 248, QWidget* parent = nullptr);

    void setArt(const QPixmap& pix);
    void setFocused(bool focused);
    const Game& game() const { return m_game; }

signals:
    void clicked(const Game& game);
    void activated(const Game& game);

protected:
    void paintEvent(QPaintEvent* event) override;
    void enterEvent(QEnterEvent* event) override;
    void leaveEvent(QEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseDoubleClickEvent(QMouseEvent* event) override;

private slots:
    void hoverTick();

private:
    void startHoverAnim();
    float hoverLevel() const;
    void paintPlaceholder(QPainter& p, int w, int h, const QRectF& target);
    void paintOverlay(QPainter& p, int w, int h);
    void paintHoverBorder(QPainter& p, const QRectF& rect, float t);

    Game m_game;
    QPixmap m_art;
    bool m_focused = false;
    float m_hprog = 0.0f;
    float m_hoverClock = 0.0f;
    QTimer m_hoverAnim;
    int m_cardW, m_cardH;
};
