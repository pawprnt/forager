#include "ui/widgets/DownloadBox.h"
#include "ui/pages/Downloads.h"
#include "ui/Theme.h"
#include "ui/Style.h"
#include "utils/Format.h"

using namespace theme::C;

DownloadBox::DownloadBox(QWidget* parent)
    : QFrame(parent)
{
    setCursor(Qt::PointingHandCursor);
    setObjectName("downloadBox");
    setStyleSheet(QStringLiteral(
        "QFrame#downloadBox { background-color: %1; border-radius: %2px; }"
        "QFrame#downloadBox:hover { background-color: %3; }")
        .arg(COLOR_2).arg(RADIUS).arg(COLOR_3));

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(12, 10, 12, 10);
    layout->setSpacing(6);

    auto* top = new QHBoxLayout();
    top->setSpacing(6);
    m_name = new QLabel("Downloading");
    style::label(m_name, TEXT, 12, 600);
    m_percent = new QLabel("0%");
    style::label(m_percent, ACCENT_2, 12, 600);
    top->addWidget(m_name);
    top->addStretch(1);
    top->addWidget(m_percent);
    layout->addLayout(top);

    m_bar = new ProgressBar(4, this);
    layout->addWidget(m_bar);

    m_detail = new QLabel();
    style::label(m_detail, TEXT_FAINT, 11);
    layout->addWidget(m_detail);

    hide();
}

void DownloadBox::mousePressEvent(QMouseEvent* event) {
    emit clicked();
    QFrame::mousePressEvent(event);
}

void DownloadBox::begin(const QString& name) {
    m_name->setText(name);
    m_percent->setText("0%");
    m_bar->setValue(0);
    m_detail->clear();
    show();
}

void DownloadBox::setProgress(double percent, const QString& stage, double speed, double done, double total) {
    m_percent->setText(QStringLiteral("%1%").arg(static_cast<int>(percent)));
    m_bar->setValue(percent);
    if (stage.toLower() == "downloading") {
        QStringList bits;
        bits << QStringLiteral("%1 / %2").arg(format::size(done), format::size(total));
        if (speed > 0)
            bits << QStringLiteral("%1/s").arg(format::size(speed));
        m_detail->setText(bits.join(" \u00b7 "));
    } else {
        m_detail->setText(QStringLiteral("%1\u2026").arg(stage));
    }
}

void DownloadBox::hideDownload() {
    hide();
}
