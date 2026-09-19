#include "ui/Theme.h"
#include "ui/Fonts.h"
#include <QApplication>
#include <QStyleFactory>
#include <QFont>
#include <QPalette>

void theme::applyTheme(QApplication* app) {
    app->setStyle(QStyleFactory::create("Fusion"));

    QFont font(fonts::UI_FONT, 10);
    app->setFont(font);

    QPalette p;
    p.setColor(QPalette::Window, QColor(C::BG));
    p.setColor(QPalette::WindowText, QColor(C::TEXT));
    p.setColor(QPalette::Base, QColor(C::COLOR_1));
    p.setColor(QPalette::AlternateBase, QColor(C::COLOR_3));
    p.setColor(QPalette::Text, QColor(C::TEXT));
    p.setColor(QPalette::Button, QColor(C::COLOR_2));
    p.setColor(QPalette::ButtonText, QColor(C::TEXT));
    p.setColor(QPalette::BrightText, QColor(C::ACCENT_1));
    p.setColor(QPalette::Light, QColor(C::COLOR_5));
    p.setColor(QPalette::Dark, Qt::black);
    p.setColor(QPalette::Mid, QColor(C::COLOR_3));
    p.setColor(QPalette::Midlight, QColor(C::COLOR_4));
    p.setColor(QPalette::Shadow, Qt::black);
    p.setColor(QPalette::Highlight, QColor(C::ACCENT_1));
    p.setColor(QPalette::HighlightedText, QColor(C::BG));
    p.setColor(QPalette::Link, QColor(C::ACCENT_1));
    p.setColor(QPalette::LinkVisited, QColor(C::ACCENT_2));
    p.setColor(QPalette::PlaceholderText, QColor(C::TEXT_DIM));
    p.setColor(QPalette::ToolTipBase, QColor(C::COLOR_2));
    p.setColor(QPalette::ToolTipText, QColor(C::TEXT));

    app->setPalette(p);
    app->setStyleSheet(stylesheet());
}

QString theme::stylesheet() {
    return QStringLiteral(R"(
        QMainWindow, QWidget {
            background-color: %1;
            color: %2;
            font-family: "Be Vietnam Pro", "DejaVu Sans", sans-serif;
        }
        QWidget:disabled { color: %3; }
        QLabel { color: %2; background: transparent; }
        QPushButton {
            background-color: %4; color: %2; border: none;
            border-radius: %6px; padding: 6px 16px; font-size: 13px; font-weight: 500;
        }
        QPushButton:hover { background-color: %5; }
        QPushButton:pressed { background-color: %10; }
        QPushButton:disabled { background-color: %4; color: %3; }
        QPushButton:focus { outline: none; }
        QLineEdit {
            background-color: %5; color: %2; border: none;
            border-radius: %6px; padding: 6px 12px; font-size: 13px;
            selection-background-color: %7; selection-color: %1;
        }
        QLineEdit:focus { border: 1px solid %7; }
        QLineEdit::placeholder { color: %3; }
        QScrollArea { border: none; background: transparent; }
        QScrollBar:vertical { background: transparent; width: 8px; margin: 2px; }
        QScrollBar::handle:vertical { background: %11; border-radius: 4px; min-height: 40px; }
        QScrollBar::handle:vertical:hover { background: %12; }
        QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0; }
        QScrollBar:horizontal { background: transparent; height: 8px; margin: 2px; }
        QScrollBar::handle:horizontal { background: %11; border-radius: 4px; min-width: 40px; }
        QScrollBar::handle:horizontal:hover { background: %12; }
        QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal { width: 0; }
        QMenu {
            background-color: %4; color: %2; border: none;
            border-radius: %6px; padding: 4px; font-size: 13px;
        }
        QMenu::item { padding: 6px 24px; border-radius: 4px; }
        QMenu::item:selected { background-color: %5; }
        QToolTip {
            background-color: %4; color: %2; border: none;
            border-radius: 4px; padding: 4px 8px;
        }
        QMessageBox { background-color: %13; }
        QMessageBox QLabel { color: %2; }
        QMessageBox QPushButton { min-width: 90px; }
        QFileDialog { background-color: %13; }
    )")
    .arg(C::BG)
    .arg(C::TEXT)
    .arg(C::TEXT_DIM)
    .arg(C::COLOR_2)
    .arg(C::COLOR_3)
    .arg(C::RADIUS)
    .arg(C::ACCENT_1)
    .arg(C::ACCENT_2)
    .arg(C::GREEN)
    .arg(C::COLOR_4)
    .arg(C::COLOR_5)
    .arg(C::COLOR_6)
    .arg(C::COLOR_1);
}
