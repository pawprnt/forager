#include "ui/widgets/Sidebar.h"
#include "ui/widgets/DownloadBox.h"
#include "ui/Theme.h"
#include "ui/Style.h"
#include "ui/Icons.h"
#include "services/IconProvider.h"
#include "library/Metadata.h"

#include <QVBoxLayout>
#include <QLineEdit>
#include <QListWidget>
#include <QLabel>
#include <QFrame>
#include <QKeyEvent>
#include <algorithm>

using namespace theme;
using namespace theme::C;

Sidebar::Sidebar(QWidget* parent)
    : QWidget(parent)
{
    setFixedWidth(240);
    style::background(this, COLOR_2);

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(12, 16, 12, 12);
    layout->setSpacing(8);

    m_search = new QLineEdit(this);
    m_search->setPlaceholderText("Search games...");
    m_search->setClearButtonEnabled(true);
    m_search->setStyleSheet(style::lineeditQss());
    connect(m_search, &QLineEdit::textChanged, this, &Sidebar::onSearch);
    layout->addWidget(m_search);

    m_list = new QListWidget(this);
    m_list->setFocusPolicy(Qt::NoFocus);
    m_list->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_list->setIconSize(QSize(22, 22));
    m_list->setSpacing(4);
    m_list->setStyleSheet(style::listQss());
    connect(m_list, &QListWidget::itemSelectionChanged, this, &Sidebar::onSelectionChanged);
    connect(m_list, &QListWidget::itemDoubleClicked, this, &Sidebar::onDoubleClicked);
    layout->addWidget(m_list, 1);

    m_downloadBox = new DownloadBox(this);
    connect(m_downloadBox, &DownloadBox::clicked, this, &Sidebar::downloadClicked);
    layout->addWidget(m_downloadBox);

    auto* panel = new QFrame(this);
    style::panel(panel, 3);
    auto* panelLayout = new QVBoxLayout(panel);
    panelLayout->setContentsMargins(10, 10, 10, 10);
    panelLayout->setSpacing(8);

    m_countLabel = new QLabel(panel);
    style::label(m_countLabel, TEXT_MUTED, 12);
    panelLayout->addWidget(m_countLabel);

    layout->addWidget(panel);
}

void Sidebar::onSearch(const QString& text) {
    m_searchText = text.trimmed().toLower();
    rebuildList();
    emit searchChanged(m_searchText);
}

void Sidebar::activateItem(QListWidgetItem* item) {
    if (!item) return;
    auto game = item->data(Qt::UserRole).value<Game>();
    if (!game.name().isEmpty())
        emit gameSelected(game);
}

void Sidebar::onSelectionChanged() {
    activateItem(m_list->currentItem());
}

void Sidebar::onDoubleClicked(QListWidgetItem* item) {
    activateItem(item);
}

void Sidebar::setGames(const QList<Game>& games) {
    m_games = games;
    std::sort(m_games.begin(), m_games.end(), [](const Game& a, const Game& b) {
        return metadata::sortKey(a).toLower() < metadata::sortKey(b).toLower();
    });
    rebuildList();
}

void Sidebar::rebuildList() {
    Game keep;
    bool hasKeep = false;
    auto* current = m_list->currentItem();
    if (current) {
        keep = current->data(Qt::UserRole).value<Game>();
        hasKeep = true;
    }

    m_list->blockSignals(true);
    m_list->clear();
    int shown = 0;
    for (const auto& g : m_games) {
        if (!m_searchText.isEmpty() && !metadata::matchesQuery(g, m_searchText))
            continue;
        auto* item = new QListWidgetItem();
        item->setText(QString(g.name()).replace("/", " / "));
        QVariant v;
        v.setValue(g);
        item->setData(Qt::UserRole, v);
        item->setToolTip(g.displayPath());
        QPixmap pix = iconprovider::loadIcon(g, false);
        if (!pix.isNull()) {
            item->setIcon(QIcon(pix));
        } else {
            item->setIcon(icons::loadIcon("box", TEXT_MUTED));
        }
        m_list->addItem(item);
        if (hasKeep && g == keep)
            m_list->setCurrentItem(item);
        ++shown;
    }

    if (!hasKeep && m_list->count())
        m_list->setCurrentRow(0);
    m_list->blockSignals(false);

    int total = m_games.size();
    m_countLabel->setText(QStringLiteral("%1 of %2 games").arg(shown).arg(total));
}

QListWidgetItem* Sidebar::findItem(const Game& game) const {
    for (int i = 0; i < m_list->count(); ++i) {
        auto* item = m_list->item(i);
        if (item->data(Qt::UserRole).value<Game>() == game)
            return item;
    }
    return nullptr;
}

void Sidebar::setGameArt(const Game& game, const QPixmap& pix) {
    if (auto* item = findItem(game))
        item->setIcon(QIcon(pix));
}

void Sidebar::selectGame(const Game& game) {
    if (auto* item = findItem(game))
        m_list->setCurrentItem(item);
}

bool Sidebar::focusNext(int direction) {
    int row = m_list->currentRow() + direction;
    if (row >= 0 && row < m_list->count()) {
        m_list->setCurrentRow(row);
        return true;
    }
    return false;
}

bool Sidebar::activateCurrent() {
    auto* item = m_list->currentItem();
    if (item) {
        onSelectionChanged();
        return true;
    }
    return false;
}

void Sidebar::beginDownload(const QString& name) {
    m_downloadBox->begin(name);
}

void Sidebar::setDownloadProgress(double percent, const QString& stage, double speed, double done, double total) {
    m_downloadBox->setProgress(percent, stage, speed, done, total);
}

void Sidebar::hideDownload() {
    m_downloadBox->hideDownload();
}
