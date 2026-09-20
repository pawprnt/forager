#pragma once

#include "core/Game.h"

#include <QWidget>
#include <QList>
#include <QPixmap>

class QLineEdit;
class QListWidget;
class QListWidgetItem;
class QLabel;
class DownloadBox;

class Sidebar : public QWidget {
    Q_OBJECT

public:
    explicit Sidebar(QWidget* parent = nullptr);

    void setGames(const QList<Game>& games);
    void setGameArt(const Game& game, const QPixmap& pix);
    void selectGame(const Game& game);
    bool focusNext(int direction);
    bool activateCurrent();

    void beginDownload(const QString& name);
    void setDownloadProgress(double percent, const QString& stage, double speed, double done, double total);
    void hideDownload();

signals:
    void gameSelected(const Game& game);
    void searchChanged(const QString& text);
    void downloadClicked();

private slots:
    void onSearch(const QString& text);
    void onSelectionChanged();
    void onDoubleClicked(QListWidgetItem* item);

private:
    void rebuildList();

    QList<Game> m_games;
    QString m_searchText;
    QLineEdit* m_search = nullptr;
    QListWidget* m_list = nullptr;
    DownloadBox* m_downloadBox = nullptr;
    QLabel* m_countLabel = nullptr;
};
