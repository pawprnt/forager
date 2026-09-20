#pragma once

#include <QMainWindow>
#include <QStackedWidget>
#include <QSet>
#include <QTimer>
#include "core/Game.h"

class Sidebar;
class TitleBar;
class GameGrid;
class GamePage;
class DownloadsPage;
class StorePage;
class RecentRow;
class LoadingSpinner;
class ControllerNav;
class GamepadNavigation;
class PlaytimeStore;
class PlaytimeTracker;
class ArtSignals;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

private slots:
    void showHome();
    void showStore();
    void showDownloads();
    void openGame(const Game& game);
    void openSettings();
    void updateProton();
    void onSearchChanged(const QString& query);
    void launchGame(const Game& game);
    void stopGame(const Game& game);
    void installGame(const Game& game);
    void cancelDownload();
    void onGamesScanned(const QList<Game>& games);
    void onGridReady(const Game& game, const QByteArray& data);
    void onIconReady(const Game& game, const QByteArray& data);
    void onHeroReady(const Game& game, const QByteArray& data);
    void onDownloadProgress(double percent, const QString& stage, double speed, double done, double total);
    void onDownloadDone(bool ok, const QString& message);
    void playTick();
    void shutdownThreads();

private:
    void setupUi();
    void wireController();
    void loadGames();
    void startArtWorker();
    void loadHeroAsync(const Game& game);
    void updateGridPanel();
    void finishLoading();

    Sidebar* m_sidebar = nullptr;
    TitleBar* m_titlebar = nullptr;
    QStackedWidget* m_content = nullptr;
    QWidget* m_home = nullptr;
    GamePage* m_gamepage = nullptr;
    DownloadsPage* m_downloadsPage = nullptr;
    StorePage* m_storePage = nullptr;
    GameGrid* m_grid = nullptr;
    RecentRow* m_recent = nullptr;
    LoadingSpinner* m_loading = nullptr;

    ControllerNav* m_controller = nullptr;
    GamepadNavigation* m_nav = nullptr;

    PlaytimeStore* m_playtimeStore = nullptr;
    PlaytimeTracker* m_playtime = nullptr;
    QTimer* m_playTimer = nullptr;

    QList<Game> m_games;
    int m_cardW = 165;
    int m_cardH = 248;
    int m_scanGeneration = 0;
    bool m_scanDone = false;
    bool m_closed = false;
    QSet<QString> m_heroDone;

    QThread* m_artThread = nullptr;
    ArtSignals* m_artSignals = nullptr;
    QThread* m_activeDownload = nullptr;
};
