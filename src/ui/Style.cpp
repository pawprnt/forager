#include "ui/Style.h"
#include "ui/Theme.h"
#include "ui/Fonts.h"
#include <QLabel>
#include <QFrame>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFont>

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
    if (kind == "quiet")
        return QStringLiteral("QPushButton { background-color: %1; color: %2; border: none; border-radius: %3px; padding: 5px 14px; } QPushButton:hover { background-color: %4; }").arg(COLOR_3, TEXT).arg(RADIUS).arg(COLOR_1);
    if (kind == "ghost")
        return QStringLiteral("QPushButton { background: transparent; color: %1; border: none; } QPushButton:hover { color: %2; }").arg(ACCENT_1, ACCENT_2);
    if (kind == "overlay")
        return QStringLiteral("QPushButton { background-color: rgba(17,17,17,170); color: %1; border: none; border-radius: %2px; padding: 8px 14px; font-size: 13px; font-weight: 600; } QPushButton:hover { background-color: rgba(17,17,17,230); }").arg(TEXT).arg(RADIUS);
    if (kind == "tab")
        return QStringLiteral("QPushButton { background-color: %1; color: #a9a9a9; border: none; border-radius: %2px; padding: 6px 16px; font-size: 14px; } QPushButton:hover { background-color: %3; color: %4; } QPushButton:checked { background-color: %5; color: %6; font-weight: 600; }").arg(COLOR_3).arg(RADIUS).arg(COLOR_4).arg(TEXT).arg(ACCENT_1).arg(TEXT);
    if (kind == "navtab")
        return QStringLiteral("QPushButton { background: transparent; border: none; border-bottom: 3px solid transparent; color: #8b929a; font-size: 14px; padding: 8px 14px 5px 14px; } QPushButton:hover { color: %1; } QPushButton:checked { color: %2; border-bottom: 3px solid %2; }").arg(TEXT).arg(ACCENT_1);
    if (kind == "navside")
        return QStringLiteral("QPushButton { background: transparent; color: %1; border: none; border-left: 3px solid transparent; border-radius: 6px; padding: 9px 12px; padding-left: 13px; font-size: 13px; text-align: left; } QPushButton:hover { color: %2; background-color: %3; } QPushButton:checked { color: %4; background-color: %3; font-weight: 600; border-left: 3px solid %4; padding-left: 10px; }").arg(TEXT_FAINT, TEXT, COLOR_3, ACCENT_1);
    if (kind == "bare")
        return QStringLiteral("QPushButton { background: transparent; border: none; border-radius: 6px; padding: 0; }");
    if (kind == "icon")
        return iconButtonQss();
    if (kind == "pill")
        return pillQss();
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

QString style::listQss() {
    return QStringLiteral("QListWidget { background: transparent; border: none; outline: none; padding-top: 4px; font-size: 12px; }"
        "QListWidget::item { padding: 3px 8px; border-radius: %1px; color: %2; }"
        "QListWidget::item:hover { background-color: %3; color: %4; }"
        "QListWidget::item:selected { background-color: rgba(102, 108, 255, 90); color: %5; }")
        .arg(RADIUS).arg(TEXT_MUTED).arg(COLOR_3).arg(TEXT).arg(ACCENT_2);
}

QString style::scrollTransparentQss() {
    return QStringLiteral("QScrollArea { background: transparent; border: none; }"
        "QScrollArea > QWidget > QWidget { background: transparent; }");
}

QString style::dialogButtonsQss() {
    return QStringLiteral(
        "QPushButton { background-color: %1; color: %2; border: 1px solid %3; "
        "border-radius: %4px; padding: 0 18px; min-height: 32px; font-size: 14px; font-weight: 600; } "
        "QPushButton:hover { background-color: %3; } "
        "QPushButton#saveButton { background-color: %5; color: %2; border: none; } "
        "QPushButton#saveButton:hover { background-color: %6; }")
        .arg(COLOR_2, TEXT, COLOR_3).arg(RADIUS).arg(ACCENT_1, ACCENT_2);
}

void style::label(QLabel* w, const QString& color, int size, int weight, const QString& extra) {
    QString s = QStringLiteral("color: %1; background: transparent;").arg(color);
    if (size > 0) s += QStringLiteral(" font-size: %1px;").arg(size);
    if (weight > 0) s += QStringLiteral(" font-weight: %1;").arg(weight);
    if (!extra.isEmpty()) s += QStringLiteral(" ") + extra;
    w->setStyleSheet(s);
}

void style::panel(QWidget* w, int level, int radius) {
    w->setStyleSheet(QStringLiteral("background-color: %1; border-radius: %2px;")
        .arg(level == 2 ? COLOR_2 : level == 3 ? COLOR_3 : COLOR_4).arg(radius));
}

void style::transparent(QWidget* w) {
    w->setStyleSheet(QStringLiteral("background: transparent;"));
}

QFrame* style::card(QWidget* parent, int colorLevel, int mh, int mv, int spacing) {
    auto* frame = new QFrame(parent);
    panel(frame, colorLevel);
    auto* lay = new QVBoxLayout(frame);
    lay->setContentsMargins(mh, mv, mh, mv);
    lay->setSpacing(spacing);
    return frame;
}

QPushButton* style::button(const QString& text, const QString& kind, QWidget* parent) {
    auto* b = new QPushButton(text, parent);
    b->setCursor(Qt::PointingHandCursor);
    b->setStyleSheet(buttonQss(kind));
    return b;
}

QLabel* style::heading(const QString& text, int px, QWidget* parent, int weight, const QString& color) {
    auto* l = new QLabel(text, parent);
    l->setFont(QFont(fonts::UI_FONT, px, weight));
    label(l, color);
    return l;
}

void style::background(QWidget* w, const QString& color) {
    w->setStyleSheet(QStringLiteral("background-color: %1;").arg(color));
}

QVBoxLayout* style::vbox(QWidget* parent, int spacing) {
    auto* lay = new QVBoxLayout(parent);
    lay->setContentsMargins(0, 0, 0, 0);
    lay->setSpacing(spacing);
    return lay;
}

QHBoxLayout* style::hbox(QWidget* parent, int spacing) {
    auto* lay = new QHBoxLayout(parent);
    lay->setContentsMargins(0, 0, 0, 0);
    lay->setSpacing(spacing);
    return lay;
}
