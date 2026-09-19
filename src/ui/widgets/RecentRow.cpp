#include "ui/widgets/RecentRow.h"
#include "ui/widgets/GameCard.h"
#include "ui/Theme.h"
#include "ui/Style.h"
#include "ui/Fonts.h"
#include <QFontMetrics>

using namespace theme::C;

static constexpr int MAX_RECENT = 8;

RecentRow::RecentRow(int cardW, int cardH, QWidget* parent)
    : QWidget(parent), m_cardW(cardW), m_cardH(cardH)
{
    auto* v = new QVBoxLayout(this);
    v->setContentsMargins(0, 0, 0, 0);
    v->setSpacing(8);

    auto* title = new QLabel("RECENTLY PLAYED");
    style::label(title, BLUE, 11, 800);
    v->addWidget(title);

    m_empty = new QLabel("Games you play will show up here.");
    style::label(m_empty, TEXT_DIM, 13);
    m_empty->setVisible(false);
    v->addWidget(m_empty);

    m_scroll = new QScrollArea();
    m_scroll->setWidgetResizable(true);
    m_scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_scroll->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_scroll->setStyleSheet(
        "QScrollArea { background: transparent; border: none; }"
        "QScrollArea > QWidget > QWidget { background: transparent; }");

    m_host = new QWidget();
    m_host->setStyleSheet("background: transparent;");
    m_row = new QHBoxLayout(m_host);
    m_row->setContentsMargins(8, 0, 8, 0);
    m_row->setSpacing(12);
    m_row->addStretch(1);
    m_scroll->setWidget(m_host);
    v->addWidget(m_scroll);
}

void RecentRow::setGames(const QList<Game>& games) {
    m_games = games;
    rebuild();
}

void RecentRow::refresh() {
    rebuild();
}

void RecentRow::setSearch(const QString& text) {
    m_searchText = text;
    setVisible(text.trimmed().isEmpty());
    rebuild();
}

void RecentRow::setCardArt(const Game& game, const QPixmap& pix) {
    for (auto& item : m_items) {
        if (item.game == game) {
            item.card->setArt(pix);
            return;
        }
    }
}

QList<GameCard*> RecentRow::cards() const {
    QList<GameCard*> result;
    for (const auto& item : m_items)
        result.append(item.card);
    return result;
}

void RecentRow::rebuild() {
    for (auto& item : m_items) {
        m_row->removeWidget(item.frame);
        item.frame->deleteLater();
    }
    m_items.clear();

    for (const auto& game : m_games)
        m_items.append(buildItem(game));

    m_empty->setVisible(m_items.isEmpty());
    m_scroll->setVisible(!m_items.isEmpty());
    if (!m_items.isEmpty()) {
        int maxH = 0;
        for (const auto& item : m_items)
            maxH = qMax(maxH, item.frame->sizeHint().height());
        m_scroll->setFixedHeight(maxH);
    }
}

RecentRow::Item RecentRow::buildItem(const Game& game) {
    auto* frame = new QWidget();
    frame->setStyleSheet("background: transparent;");
    auto* v = new QVBoxLayout(frame);
    v->setContentsMargins(0, 0, 0, 0);
    v->setSpacing(6);

    auto* card = new GameCard(game, m_cardW, m_cardH);
    connect(card, &GameCard::clicked, this, &RecentRow::gameClicked);
    v->addWidget(card, 0, Qt::AlignHCenter);

    QString nameText = elide(QString(game.name()).replace("/", " / "));
    auto* nameLabel = new QLabel(nameText);
    nameLabel->setFixedWidth(m_cardW);
    nameLabel->setAlignment(Qt::AlignHCenter);
    style::label(nameLabel, TEXT, 12);
    nameLabel->setToolTip(game.name());
    v->addWidget(nameLabel);

    auto* timeLabel = new QLabel("0 h");
    timeLabel->setFixedWidth(m_cardW);
    timeLabel->setAlignment(Qt::AlignHCenter);
    style::label(timeLabel, TEXT_DIM, 11);
    v->addWidget(timeLabel);

    m_row->insertWidget(m_row->count() - 1, frame);
    return {frame, card, game};
}

QString RecentRow::elide(const QString& text) const {
    QFont font(fonts::UI_FONT, 12);
    QFontMetrics fm(font);
    return fm.elidedText(text, Qt::ElideRight, m_cardW);
}
