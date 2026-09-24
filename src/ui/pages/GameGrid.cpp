#include "ui/pages/GameGrid.h"
#include "ui/widgets/GameCard.h"
#include "ui/Theme.h"
#include "ui/Style.h"
#include "library/Metadata.h"
#include "artwork/Pipeline.h"
#include <QScrollBar>
#include <QEvent>
#include <algorithm>

using namespace theme;
using namespace theme::C;

static constexpr int GRID_MARGIN = 8;
static constexpr int GRID_MIN_GAP = 12;
static constexpr int GRID_V_GAP = 16;

GameGrid::GameGrid(int cardW, int cardH, QWidget* parent)
    : QWidget(parent), m_cardW(cardW), m_cardH(cardH)
{
    style::transparent(this);

    auto* v = style::vbox(this);

    m_emptyLabel = new QLabel("No games found.", this);
    m_emptyLabel->setAlignment(Qt::AlignCenter);
    style::label(m_emptyLabel, C::TEXT_DIM, 14);
    m_emptyLabel->setStyleSheet(m_emptyLabel->styleSheet() + "padding: 60px;");
    v->addWidget(m_emptyLabel);

    m_scroll = new QScrollArea(this);
    m_scroll->setWidgetResizable(true);
    m_scroll->setStyleSheet(style::scrollTransparentQss());
    m_scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    m_gridHost = new QWidget();
    style::transparent(m_gridHost);
    m_grid = new QGridLayout(m_gridHost);
    m_grid->setContentsMargins(0, 0, 0, 0);
    m_grid->setSpacing(GRID_V_GAP);
    m_grid->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    m_scroll->setWidget(m_gridHost);
    v->addWidget(m_scroll, 1);

    m_scroll->viewport()->installEventFilter(this);
}

void GameGrid::setGames(const QList<Game>& games) {
    m_games = games;
    rebuildCards();
}

void GameGrid::setSearch(const QString& text) {
    m_searchText = text;
    rebuildCards();
}

void GameGrid::setCardSize(int cardW, int cardH) {
    m_cardW = cardW;
    m_cardH = cardH;
    for (auto* card : m_cards) card->setFixedSize(cardW, cardH);
    relayoutCards();
}

int GameGrid::count() const { return m_cards.size(); }

void GameGrid::setCardArt(const Game& game, const QPixmap& pix) {
    for (auto* card : m_cards) {
        if (card->game() == game) { card->setArt(pix); return; }
    }
}

void GameGrid::focusIndex(int index) {
    if (m_cards.isEmpty()) return;
    index = std::clamp(index, 0, static_cast<int>(m_cards.size()) - 1);
    for (int i = 0; i < m_cards.size(); ++i)
        m_cards[i]->setFocused(i == index);
    m_cardIndex = index;
    m_scroll->ensureWidgetVisible(m_cards[index], 40, 40);
}

void GameGrid::rebuildCards() {
    for (auto* card : m_cards) {
        m_grid->removeWidget(card);
        card->deleteLater();
    }
    m_cards.clear();
    m_cardIndex = 0;

    QList<Game> filtered = metadata::filterGames(m_games, m_searchText);
    std::sort(filtered.begin(), filtered.end(), [](const Game& a, const Game& b) {
        return metadata::sortKey(a) < metadata::sortKey(b);
    });

    for (const auto& game : filtered) {
        auto* card = new GameCard(game, m_cardW, m_cardH);
        connect(card, &GameCard::clicked, this, &GameGrid::cardClicked);
        connect(card, &GameCard::activated, this, &GameGrid::cardActivated);
        m_cards.append(card);
    }

    m_emptyLabel->setVisible(m_cards.isEmpty());
    m_scroll->setVisible(!m_cards.isEmpty());
    relayoutCards();
    loadCardArt();
    emit layoutChanged();
}

void GameGrid::loadCardArt() {
    for (auto* card : m_cards) {
        QPixmap pix = art::loadGrid(card->game(), false);
        if (!pix.isNull())
            card->setArt(pix);
    }
}

void GameGrid::relayoutCards() {
    if (m_cards.isEmpty()) return;

    while (m_grid->count() > 0) {
        auto* item = m_grid->takeAt(0);
        if (item->widget()) m_grid->removeWidget(item->widget());
        delete item;
    }

    int viewportW = m_scroll->viewport()->width();
    auto* sb = m_scroll->verticalScrollBar();
    if (sb) viewportW -= sb->sizeHint().width();

    int avail = std::max(1, viewportW - 2 * GRID_MARGIN);
    int cols = std::max(1, (avail + GRID_MIN_GAP) / (m_cardW + GRID_MIN_GAP));
    cols = std::min(cols, static_cast<int>(m_cards.size()));

    int used = cols * m_cardW + (cols - 1) * GRID_MIN_GAP;
    int remaining = avail - used;
    int gap = (cols > 1 && remaining > 0) ? GRID_MIN_GAP + remaining / (cols - 1) : GRID_MIN_GAP;

    m_grid->setContentsMargins(GRID_MARGIN, 0, GRID_MARGIN, 0);
    m_grid->setHorizontalSpacing(gap);
    m_grid->setVerticalSpacing(GRID_V_GAP);

    for (int i = 0; i < m_cards.size(); ++i)
        m_grid->addWidget(m_cards[i], i / cols, i % cols);

    emit layoutChanged();
}

bool GameGrid::eventFilter(QObject* obj, QEvent* event) {
    if (obj == m_scroll->viewport() && event->type() == QEvent::Resize)
        relayoutCards();
    return QWidget::eventFilter(obj, event);
}
