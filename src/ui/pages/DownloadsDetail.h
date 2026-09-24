#pragma once

#include "ui/pages/Downloads.h"
#include "ui/Theme.h"
#include "ui/Style.h"
#include "ui/Icons.h"
#include <QPainter>
#include <QLinearGradient>

using namespace theme;
using namespace theme::C;

inline QLabel* makeChip(const QString& iconName, const QString& iconColor, int iconPx, int boxPx) {
    auto* chip = new QLabel();
    chip->setPixmap(icons::loadIcon(iconName, iconColor).pixmap(iconPx, iconPx));
    chip->setFixedSize(boxPx, boxPx);
    chip->setAlignment(Qt::AlignCenter);
    style::panel(chip, 3, 6);
    return chip;
}

class DownloadsPage::Banner : public QWidget {
public:
    explicit Banner(QWidget* parent = nullptr) : QWidget(parent) {
        setMinimumHeight(230);
        m_watermark = icons::loadIcon("download", C::TEXT).pixmap(170, 170);

        auto* layout = style::vbox(this);

        auto* info = new QHBoxLayout();
        info->setContentsMargins(28, 26, 28, 0);
        info->setSpacing(10);
        auto* col = new QVBoxLayout();
        col->setSpacing(4);
        m_title = new QLabel();
        style::label(m_title, C::TEXT, 20, 700);
        m_status = new QLabel();
        style::label(m_status, C::ACCENT_1, 13, 600);
        col->addWidget(m_title);
        col->addWidget(m_status);
        info->addLayout(col);
        info->addStretch(1);
        layout->addLayout(info);
        layout->addStretch(1);

        m_bar = new ProgressBar(4, this);
        layout->addWidget(m_bar);
    }

    void setTitle(const QString& text) { m_title->setText(text); }
    void setStatus(const QString& text) { m_status->setText(text); }
    void setBar(double percent) { m_bar->setValue(percent); }

protected:
    void paintEvent(QPaintEvent*) override {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);
        int w = width(), h = height();

        QLinearGradient base(0, 0, 0, h);
        base.setColorAt(0.0, QColor("#222833"));
        base.setColorAt(1.0, QColor("#12161c"));
        p.fillRect(0, 0, w, h, base);

        int side = qMin(w, h) / 2;
        if (!m_watermark.isNull()) {
            p.setOpacity(0.10);
            p.drawPixmap((w - side) / 2, static_cast<int>(h * 0.42) - side / 2, side, side, m_watermark);
            p.setOpacity(1.0);
        }

        QLinearGradient fade(0, 0, w, 0);
        QColor transparent(C::COLOR_2);
        transparent.setAlpha(0);
        fade.setColorAt(0.15, transparent);
        fade.setColorAt(0.90, QColor(C::COLOR_2));
        p.fillRect(0, 0, w, h, fade);
    }

private:
    QLabel* m_title = nullptr;
    QLabel* m_status = nullptr;
    ProgressBar* m_bar = nullptr;
    QPixmap m_watermark;
};

class DownloadsPage::StatItem : public QWidget {
public:
    explicit StatItem(const QString& iconName, const QString& caption, bool accent = false, QWidget* parent = nullptr)
        : QWidget(parent)
    {
        auto* lay = style::hbox(this, 10);

        auto* chip = makeChip(iconName, C::ACCENT_1, 18, 36);

        auto* text = new QVBoxLayout();
        text->setSpacing(1);
        auto* cap = new QLabel(caption);
        style::label(cap, TEXT_FAINT, 11);
        m_value = new QLabel("\u2014");
        style::label(m_value, accent ? C::ACCENT_1 : C::TEXT, 15, 700);
        text->addWidget(cap);
        text->addWidget(m_value);

        lay->addWidget(chip);
        lay->addLayout(text);
        lay->addStretch(1);
    }

    void setValue(const QString& text) { m_value->setText(text); }

private:
    QLabel* m_value = nullptr;
};
