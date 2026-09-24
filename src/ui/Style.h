#pragma once

#include <QString>

class QWidget;
class QLabel;
class QFrame;
class QPushButton;
class QVBoxLayout;
class QHBoxLayout;

namespace style {
    QString buttonQss(const QString& kind = "primary");
    QString lineeditQss();
    QString iconButtonQss();
    QString pillQss();
    QString toolbuttonQss();
    QString listQss();
    QString scrollTransparentQss();
    QString dialogButtonsQss();
    void label(QLabel* w, const QString& color = "#ffffff", int size = -1, int weight = -1, const QString& extra = {});
    void panel(QWidget* w, int level = 2, int radius = 8);
    void transparent(QWidget* w);
    QFrame* card(QWidget* parent = nullptr, int colorLevel = 2, int mh = 12, int mv = 12, int spacing = 8);
    QPushButton* button(const QString& text, const QString& kind, QWidget* parent = nullptr);
    QLabel* heading(const QString& text, int px, QWidget* parent = nullptr, int weight = 700, const QString& color = "#ffffff");
    void background(QWidget* w, const QString& color);
    QVBoxLayout* vbox(QWidget* parent, int spacing = 0);
    QHBoxLayout* hbox(QWidget* parent, int spacing = 0);
} // namespace style
