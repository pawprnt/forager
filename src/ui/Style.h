#pragma once

#include <QString>

class QWidget;
class QLabel;

namespace style {
    QString buttonQss(const QString& kind = "primary");
    QString lineeditQss();
    QString iconButtonQss();
    QString pillQss();
    QString toolbuttonQss();
    void label(QLabel* w, const QString& color = "#ffffff", int size = -1, int weight = -1);
    void panel(QWidget* w, int level = 2, int radius = 8);
} // namespace style
