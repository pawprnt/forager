#pragma once

#include <QWidget>
#include <QScrollArea>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QVector>
#include "core/Game.h"

class GameCard;

class GameGrid : public QWidget {
    Q_OBJECT

public:
    explicit GameGrid(int cardW, int cardH, QWidget* parent = nullptr);

    void setGames(const QList<Game>& games);
    void setSearch(const QString& text);
    void setCardSize(int cardW, int cardH);
    void setCardArt(const Game& game, const QPixmap& pix);
    void focusIndex(int index);
    int count() const;
    int currentIndex() const { return m_cardIndex; }

signals:
    void cardClicked(const Game& game);
    void cardActivated(const Game& game);
    void layoutChanged();

private slots:
    void relayoutCards();

protected:
    bool eventFilter(QObject* obj, QEvent* event) override;

private:
    void rebuildCards();
    void loadCardArt();

    QList<Game> m_games;
    QList<GameCard*> m_cards;
    int m_cardIndex = 0;
    int m_cardW, m_cardH;
    QString m_searchText;

    QScrollArea* m_scroll = nullptr;
    QWidget* m_gridHost = nullptr;
    QGridLayout* m_grid = nullptr;
    QLabel* m_emptyLabel = nullptr;
};
