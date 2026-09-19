#pragma once

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QScrollArea>
#include <QVector>
#include "core/Game.h"

class GameCard;

class RecentRow : public QWidget {
    Q_OBJECT

public:
    explicit RecentRow(int cardW = 120, int cardH = 180, QWidget* parent = nullptr);

    void setGames(const QList<Game>& games);
    void refresh();
    void setSearch(const QString& text);
    void setCardArt(const Game& game, const QPixmap& pix);
    QList<GameCard*> cards() const;

signals:
    void gameClicked(const Game& game);

private:
    struct Item {
        QWidget* frame;
        GameCard* card;
        Game game;
    };

    void rebuild();
    Item buildItem(const Game& game);
    QString elide(const QString& text) const;

    QList<Game> m_games;
    QList<Item> m_items;
    int m_cardW, m_cardH;
    QString m_searchText;

    QScrollArea* m_scroll = nullptr;
    QWidget* m_host = nullptr;
    QHBoxLayout* m_row = nullptr;
    QLabel* m_empty = nullptr;
};
