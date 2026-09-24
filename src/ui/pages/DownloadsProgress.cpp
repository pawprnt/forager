#include "ui/pages/Downloads.h"
#include "ui/pages/DownloadsDetail.h"
#include "ui/Theme.h"
#include "ui/Style.h"
#include "utils/Format.h"
#include <QPainter>

using namespace theme;
using namespace theme::C;

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

void DownloadsPage::setStatus(const QString& status) {
    m_banner->setStatus(status);
    m_itemStatus->setText(status);
}

void DownloadsPage::begin(const QString& name) {
    m_banner->show();
    m_item->show();
    m_empty->hide();
    m_banner->setTitle(name);
    setStatus("Waiting to start\u2026");
    m_banner->setBar(0);
    m_itemName->setText(name);
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

    setStatus(status);
    m_banner->setBar(percent);
    m_itemBar->setValue(percent);

    if (stage.toLower() == "downloading" && speed > 0) {
        m_speedStat->setValue(QStringLiteral("%1/s").arg(format::size(speed)));
        double remaining = total - done;
        m_timeStat->setValue(remaining > 0 ? formatEta(remaining / speed) : "\u2014");
    } else {
        m_speedStat->setValue("\u2014");
        m_timeStat->setValue("\u2014");
    }
}

void DownloadsPage::finish(const QString& status) {
    m_itemCancel->hide();
    setStatus(status);
    m_banner->setBar(100);
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
