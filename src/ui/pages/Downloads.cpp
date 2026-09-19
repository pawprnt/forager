#include "ui/pages/Downloads.h"
#include "ui/Theme.h"
#include "ui/Style.h"
#include "ui/Icons.h"
#include "ui/Fonts.h"
#include <QPainter>
#include <QLinearGradient>
#include <QFileInfo>

using namespace theme;
using namespace theme::C;

static QString formatSize(double num) {
    const char* units[] = {"B", "KB", "MB", "GB", "TB"};
    for (int i = 0; i < 5; ++i) {
        if (num < 1024)
            return QStringLiteral("%1 %2").arg(num, 0, 'f', 1).arg(units[i]);
        num /= 1024;
    }
    return QStringLiteral("%1 PB").arg(num, 0, 'f', 1);
}

static QString formatEta(double seconds) {
    int s = qMax(0, static_cast<int>(seconds));
    int m = s / 60;
    s = s % 60;
    if (m > 0)
        return QStringLiteral("%1m %2s").arg(m).arg(s, 2, 10, QChar('0'));
    return QStringLiteral("%1s").arg(s);
}

// -- ProgressBar -----------------------------------------------------------

ProgressBar::ProgressBar(int height, QWidget* parent)
    : QWidget(parent), m_height(height)
{
    setFixedHeight(height);
    setMinimumWidth(40);
}

void ProgressBar::setValue(double value) {
    m_value = qBound(0.0, value, 100.0);
    update();
}

void ProgressBar::paintEvent(QPaintEvent*) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    auto r = QRectF(rect()).adjusted(0.5, 0.5, -0.5, -0.5);
    double radius = r.height() / 2.0;
    p.setPen(Qt::NoPen);
    p.setBrush(QColor(C::BG));
    p.drawRoundedRect(r, radius, radius);
    if (m_value > 0) {
        double w = qMax(r.height(), r.width() * m_value / 100.0);
        p.setBrush(QColor(C::ACCENT_1));
        p.drawRoundedRect(QRectF(r.left(), r.top(), w, r.height()), radius, radius);
    }
}

// -- DownloadsPage ---------------------------------------------------------

class DownloadsPage::Banner : public QWidget {
public:
    explicit Banner(QWidget* parent = nullptr) : QWidget(parent) {
        setMinimumHeight(230);
        m_watermark = icons::loadIcon("download", C::TEXT).pixmap(170, 170);

        auto* layout = new QVBoxLayout(this);
        layout->setContentsMargins(0, 0, 0, 0);
        layout->setSpacing(0);

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
        auto* lay = new QHBoxLayout(this);
        lay->setContentsMargins(0, 0, 0, 0);
        lay->setSpacing(10);

        auto* chip = new QLabel();
        chip->setPixmap(icons::loadIcon(iconName, C::ACCENT_1).pixmap(18, 18));
        chip->setFixedSize(36, 36);
        chip->setAlignment(Qt::AlignCenter);
        chip->setStyleSheet(QStringLiteral("background-color: %1; border-radius: 6px;").arg(C::COLOR_3));

        auto* text = new QVBoxLayout();
        text->setSpacing(1);
        auto* cap = new QLabel(caption);
        style::label(cap, "#b8bcbf", 11);
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

DownloadsPage::DownloadsPage(QWidget* parent)
    : QWidget(parent)
{
    setStyleSheet(QStringLiteral("background-color: %1;").arg(C::BG));
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(12, 8, 12, 12);
    layout->setSpacing(12);

    auto* header = new QHBoxLayout();
    header->setContentsMargins(12, 8, 12, 0);
    auto* title = new QLabel("Downloads");
    QFont titleFont(fonts::UI_FONT, 20, QFont::Bold);
    title->setFont(titleFont);
    style::label(title, C::TEXT);
    auto* gear = new QPushButton();
    gear->setIcon(icons::loadIcon("settings", C::TEXT));
    gear->setIconSize(QSize(18, 18));
    gear->setFixedSize(28, 28);
    gear->setCursor(Qt::PointingHandCursor);
    gear->setToolTip("Download settings");
    gear->setStyleSheet("QPushButton { background: transparent; border: none; border-radius: 6px; padding: 0; }");
    connect(gear, &QPushButton::clicked, this, &DownloadsPage::settingsRequested);
    header->addWidget(title);
    header->addStretch(1);
    header->addWidget(gear);
    layout->addLayout(header);

    m_banner = new Banner();
    layout->addWidget(m_banner);

    auto* stats = new QHBoxLayout();
    stats->setContentsMargins(12, 4, 12, 0);
    stats->setSpacing(28);
    m_speedStat = new StatItem("download", "Download speed", true);
    m_timeStat = new StatItem("clock-rotate-right", "Time remaining");
    m_spaceStat = new StatItem("floppy-disk", "Available space");
    stats->addWidget(m_speedStat);
    stats->addWidget(m_timeStat);
    stats->addWidget(m_spaceStat);
    stats->addStretch(1);
    layout->addLayout(stats);

    auto* queue = new QVBoxLayout();
    queue->setContentsMargins(12, 8, 12, 0);
    queue->setSpacing(8);
    auto* qheader = new QLabel("Updates");
    QFont qheaderFont(fonts::UI_FONT, 15, QFont::Bold);
    qheader->setFont(qheaderFont);
    style::label(qheader, C::TEXT);
    queue->addWidget(qheader);

    m_item = new QFrame();
    m_item->setObjectName("queueItem");
    style::panel(m_item, 2);
    auto* itemLay = new QHBoxLayout(m_item);
    itemLay->setContentsMargins(16, 12, 16, 12);
    itemLay->setSpacing(14);

    auto* chip = new QLabel();
    chip->setPixmap(icons::loadIcon("download", C::ACCENT_2).pixmap(20, 20));
    chip->setFixedSize(34, 34);
    chip->setAlignment(Qt::AlignCenter);
    chip->setStyleSheet(QStringLiteral("background-color: %1; border-radius: 6px;").arg(C::COLOR_3));
    itemLay->addWidget(chip);

    auto* col = new QVBoxLayout();
    col->setSpacing(2);
    m_itemName = new QLabel();
    style::label(m_itemName, C::TEXT, 13, 600);
    m_itemStatus = new QLabel();
    style::label(m_itemStatus, "#b8bcbf");
    col->addWidget(m_itemName);
    col->addWidget(m_itemStatus);
    itemLay->addLayout(col);
    itemLay->addStretch(1);

    m_itemBar = new ProgressBar(4, this);
    m_itemBar->setFixedWidth(220);
    itemLay->addWidget(m_itemBar);

    m_itemCancel = new QPushButton("Cancel");
    m_itemCancel->setStyleSheet(QStringLiteral(
        "QPushButton { background-color: %1; color: %2; border: none; border-radius: %3px; padding: 5px 14px; } "
        "QPushButton:hover { background-color: %4; }")
        .arg(C::COLOR_3, C::TEXT).arg(C::RADIUS).arg(C::COLOR_1));
    connect(m_itemCancel, &QPushButton::clicked, this, &DownloadsPage::cancelRequested);
    itemLay->addWidget(m_itemCancel);
    queue->addWidget(m_item);

    m_empty = new QLabel("No active downloads");
    style::label(m_empty, C::TEXT_DIM, 13);
    m_empty->setAlignment(Qt::AlignCenter);
    queue->addWidget(m_empty);

    layout->addLayout(queue);
    layout->addStretch(1);
    setIdle();
}

void DownloadsPage::refreshSpace() {
    m_spaceStat->setValue("\u2014");
}

void DownloadsPage::setIdle() {
    m_banner->hide();
    m_item->hide();
    m_empty->show();
    m_speedStat->setValue("\u2014");
    m_timeStat->setValue("\u2014");
    refreshSpace();
}

void DownloadsPage::begin(const QString& name) {
    m_banner->show();
    m_item->show();
    m_empty->hide();
    m_banner->setTitle(name);
    m_banner->setStatus("Waiting to start\u2026");
    m_banner->setBar(0);
    m_itemName->setText(name);
    m_itemStatus->setText("Waiting to start\u2026");
    m_itemBar->setValue(0);
    m_itemCancel->show();
    m_speedStat->setValue("\u2014");
    m_timeStat->setValue("\u2014");
    refreshSpace();
}

void DownloadsPage::setProgress(double percent, const QString& stage, double speed, double done, double total) {
    QString status;
    if (stage.toLower() == "downloading")
        status = QStringLiteral("Downloading \u00b7 %1%").arg(static_cast<int>(percent));
    else
        status = QStringLiteral("%1\u2026 \u00b7 %2%").arg(stage).arg(static_cast<int>(percent));

    m_banner->setStatus(status);
    m_banner->setBar(percent);
    m_itemStatus->setText(status);
    m_itemBar->setValue(percent);

    if (stage.toLower() == "downloading" && speed > 0) {
        m_speedStat->setValue(QStringLiteral("%1/s").arg(formatSize(speed)));
        double remaining = total - done;
        m_timeStat->setValue(remaining > 0 ? formatEta(remaining / speed) : "\u2014");
    } else {
        m_speedStat->setValue("\u2014");
        m_timeStat->setValue("\u2014");
    }
}

void DownloadsPage::finish(const QString& status) {
    m_itemCancel->hide();
    m_banner->setStatus(status);
    m_banner->setBar(100);
    m_itemStatus->setText(status);
    m_itemBar->setValue(100);
    m_speedStat->setValue("\u2014");
    m_timeStat->setValue("\u2014");
}

void DownloadsPage::complete(const QString& version) {
    finish(version.isEmpty() ? "Completed" : QStringLiteral("Completed\u2014%1").arg(version));
}

void DownloadsPage::failed(const QString& error) {
    finish(QStringLiteral("Failed: %1").arg(error));
}

void DownloadsPage::cancelled() {
    finish("Download cancelled");
}
