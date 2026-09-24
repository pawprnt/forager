#include "ui/pages/Downloads.h"
#include "ui/pages/DownloadsDetail.h"
#include "ui/Theme.h"
#include "ui/Style.h"
#include "ui/Icons.h"

using namespace theme;
using namespace theme::C;

// -- DownloadsPage ---------------------------------------------------------

DownloadsPage::DownloadsPage(QWidget* parent)
    : QWidget(parent)
{
    style::background(this, C::BG);
    auto* layout = style::vbox(this, 12);
    layout->setContentsMargins(12, 8, 12, 12);

    auto* header = new QHBoxLayout();
    header->setContentsMargins(12, 8, 12, 0);
    auto* title = style::heading("Downloads", 20);
    auto* gear = style::button(QString(), "bare");
    gear->setIcon(icons::loadIcon("settings", C::TEXT));
    gear->setIconSize(QSize(18, 18));
    gear->setFixedSize(28, 28);
    gear->setToolTip("Download settings");
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
    auto* qheader = style::heading("Updates", 15);
    queue->addWidget(qheader);

    m_item = new QFrame();
    m_item->setObjectName("queueItem");
    style::panel(m_item, 2);
    auto* itemLay = new QHBoxLayout(m_item);
    itemLay->setContentsMargins(16, 12, 16, 12);
    itemLay->setSpacing(14);

    itemLay->addWidget(makeChip("download", C::ACCENT_2, 20, 34));

    auto* col = new QVBoxLayout();
    col->setSpacing(2);
    m_itemName = new QLabel();
    style::label(m_itemName, C::TEXT, 13, 600);
    m_itemStatus = new QLabel();
    style::label(m_itemStatus, TEXT_FAINT);
    col->addWidget(m_itemName);
    col->addWidget(m_itemStatus);
    itemLay->addLayout(col);
    itemLay->addStretch(1);

    m_itemBar = new ProgressBar(4, this);
    m_itemBar->setFixedWidth(220);
    itemLay->addWidget(m_itemBar);

    m_itemCancel = style::button("Cancel", "quiet");
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
