#include "ui/Style.h"
#include "ui/Theme.h"
#include <QLabel>

using namespace theme::C;

QString style::buttonQss(const QString& kind) {
    QString base = QStringLiteral("QPushButton { border: none; border-radius: %1px; ").arg(RADIUS);
    if (kind == "play")
        return base + QStringLiteral("background-color: %1; color: %2; } QPushButton:hover { background-color: %3; } QPushButton:disabled { background-color: %4; color: %5; }").arg(GREEN, TEXT, GREEN_HOVER, COLOR_3, TEXT_DIM);
    if (kind == "running")
        return base + QStringLiteral("background-color: %1; color: %2; } QPushButton:hover { background-color: %3; }").arg(COLOR_3, TEXT, COLOR_4);
    if (kind == "primary")
        return base + QStringLiteral("background-color: %1; color: %2; } QPushButton:hover { background-color: %3; } QPushButton:disabled { background-color: %4; color: %5; }").arg(ACCENT_1, TEXT, ACCENT_2, COLOR_2, TEXT_DIM);
    if (kind == "secondary")
        return base + QStringLiteral("background-color: %1; color: %2; border: 1px solid %3; } QPushButton:hover { background-color: %4; }").arg(COLOR_2, TEXT, COLOR_3, COLOR_3);
    if (kind == "ghost")
        return QStringLiteral("QPushButton { background: transparent; color: %1; border: none; } QPushButton:hover { color: %2; }").arg(ACCENT_1, ACCENT_2);
    return {};
}

QString style::lineeditQss() {
    return QStringLiteral("QLineEdit { background-color: %1; border: none; border-radius: %2px; padding: 7px 12px; font-size: 13px; } QLineEdit:focus { border: 1px solid %3; }").arg(COLOR_3).arg(RADIUS).arg(ACCENT_1);
}

QString style::iconButtonQss() {
    return QStringLiteral("QPushButton { background-color: %1; border: none; border-radius: %2px; } QPushButton:hover { background-color: %3; }").arg(COLOR_2).arg(RADIUS).arg(COLOR_3);
}

QString style::pillQss() {
    return QStringLiteral("QPushButton { color: %1; background: transparent; border: 1px solid %1; border-radius: %2px; padding: 4px 10px; font-size: 11px; } QPushButton:hover { background-color: %3; }").arg(YELLOW).arg(RADIUS).arg(COLOR_3);
}

QString style::toolbuttonQss() {
    return QStringLiteral("QToolButton { color: %1; background: transparent; border: none; border-radius: %2px; padding: 6px 10px; } QToolButton:hover { background-color: %3; } QToolButton::menu-indicator { image: none; }").arg(ACCENT_1).arg(RADIUS).arg(COLOR_3);
}

void style::label(QLabel* w, const QString& color, int size, int weight) {
    QString s = QStringLiteral("color: %1; background: transparent;").arg(color);
    if (size > 0) s += QStringLiteral(" font-size: %1px;").arg(size);
    if (weight > 0) s += QStringLiteral(" font-weight: %1;").arg(weight);
    w->setStyleSheet(s);
}

void style::panel(QWidget* w, int level, int radius) {
    w->setStyleSheet(QStringLiteral("background-color: %1; border-radius: %2px;")
        .arg(level == 2 ? COLOR_2 : level == 3 ? COLOR_3 : COLOR_4).arg(radius));
}
