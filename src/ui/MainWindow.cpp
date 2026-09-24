#include "ui/MainWindow.h"
#include "ui/widgets/Sidebar.h"
#include "ui/widgets/TitleBar.h"
#include "ui/widgets/RecentRow.h"
#include "ui/widgets/LoadingSpinner.h"
#include "ui/widgets/ControllerNav.h"
#include "ui/pages/GameGrid.h"
#include "ui/pages/GamePage.h"
#include "ui/pages/Downloads.h"
#include "ui/pages/Store.h"
#include "ui/Workers.h"
#include "ui/Theme.h"
#include "ui/dialogs/SettingsDialog.h"
#include "core/Config.h"
#include "services/SteamGridDB.h"
#include "library/Launcher.h"
#include "library/Playtime.h"

#include <QMessageBox>
#include <QApplication>
#include <QThread>

using namespace theme;
using namespace theme::C;

template <typename W, typename Wire>
W* MainWindow::spawnWorker(W* w, Wire&& wire) {
    wire(w);
    w->start();
    return w;
}

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    Config::instance().load();
    SteamGridDB::instance().loadToken();
    setWindowTitle("forager");
    resize(1280, 720);

    auto [w, h] = resolveCardSize(Config::instance().displaySize());
    m_cardW = w;
    m_cardH = h;

    m_playtimeStore = new PlaytimeStore({}, this);
    m_playtime = new PlaytimeTracker(m_playtimeStore, this);

    setupUi();
    wireController();

    connect(qApp, &QApplication::aboutToQuit, this, &MainWindow::shutdownThreads);

    m_playTimer = new QTimer(this);
    connect(m_playTimer, &QTimer::timeout, this, &MainWindow::playTick);
    m_playTimer->start(15000);

    QTimer::singleShot(50, this, &MainWindow::loadGames);
}

MainWindow::~MainWindow()
{
    shutdownThreads();
    const auto threads = findChildren<QThread*>();
    for (QThread* t : threads) {
        if (t->isRunning()) {
            t->requestInterruption();
            t->quit();
            t->wait(2000);
        }
    }
}

void MainWindow::wireController()
{
    m_controller = new ControllerNav(this);

    GamepadNavigation::Callbacks cb;
    cb.isOnHome = [this]() -> bool { return m_content->currentWidget() == m_home; };
    cb.isOnGamepage = [this]() -> bool { return m_content->currentWidget() == m_gamepage; };
    cb.focusedGame = [this]() -> Game {
        int idx = m_grid->currentIndex();
        if (idx >= 0 && idx < m_games.size()) return m_games[idx];
        return {};
    };
    cb.gamepageGame = [this]() -> Game { return {}; };
    cb.openGame = [this](const Game& g) { openGame(g); };
    cb.launchGame = [this](const Game& g) { launchGame(g); };
    cb.showHome = [this]() { showHome(); };
    cb.moveFocus = [this](int dir) {
        int idx = m_grid->currentIndex() + dir;
        m_grid->focusIndex(idx);
    };
    cb.columnCount = [this]() -> int {
        if (m_grid->count() <= 0) return 1;
        int avail = m_content->width() - 48;
        return qMax(1, (avail + 12) / (m_cardW + 12));
    };
    cb.setHint = [this](const QString& text) { m_titlebar->setControllerHint(text); };

    m_nav = new GamepadNavigation(m_controller, std::move(cb), this);
}

void MainWindow::loadGames()
{
    m_loading->show();
    m_scanDone = false;
    ++m_scanGeneration;

    const auto workers = findChildren<ScanWorker*>();
    for (ScanWorker* old : workers) {
        if (old->isRunning()) {
            old->requestInterruption();
            old->wait(1000);
        }
        old->deleteLater();
    }

    int gen = m_scanGeneration;
    spawnWorker(new ScanWorker(this), [this, gen](ScanWorker* worker) {
        connect(worker, &ScanWorker::done, this, [this, gen](const QList<Game>& games) {
            if (gen != m_scanGeneration) return;
            onGamesScanned(games);
        });
        connect(worker, &QThread::finished, worker, &QObject::deleteLater);
    });
}

void MainWindow::onGamesScanned(const QList<Game>& games)
{
    m_games = games;
    m_scanDone = true;
    finishLoading();
}

void MainWindow::finishLoading()
{
    m_loading->hide();
    m_sidebar->setGames(m_games);
    m_grid->setGames(m_games);

    QList<Game> recent = m_playtime->recentlyPlayed(m_games);
    m_recent->setGames(recent);

    startArtWorker();
}

void MainWindow::launchGame(const Game& game)
{
    if (game.name().isEmpty()) return;

    auto proc = launcher::launch(game);
    if (!proc) {
        QMessageBox::warning(this, "Launch Failed",
            QStringLiteral("Could not launch '%1'.").arg(game.name()));
        return;
    }

    QProcess* rawProc = proc.release();
    rawProc->setParent(this);
    m_playtime->begin(game, rawProc);
    m_gamepage->setRunning(true);
    m_recent->setGames(m_playtime->recentlyPlayed(m_games));
}

void MainWindow::stopGame(const Game& game)
{
    m_playtime->stop(game);
    m_gamepage->setRunning(false);
    m_recent->setGames(m_playtime->recentlyPlayed(m_games));
}

void MainWindow::installGame(const Game& game)
{
    if (game.appId().isEmpty()) return;
    m_sidebar->beginDownload(game.name());
    m_downloadsPage->begin(game.name());
    showDownloads();

    QString dest = Config::instance().gamesDir() + "/" + game.name();
    m_activeDownload = spawnWorker(
        new DownloadWorker("steam", game.appId(), dest, this),
        [this](DownloadWorker* worker) {
            connect(worker, &DownloadWorker::progress, this, &MainWindow::onDownloadProgress);
            connect(worker, &DownloadWorker::done, this, &MainWindow::onDownloadDone);
        });
}

void MainWindow::onDownloadProgress(double percent, const QString& stage, double speed, double done, double total)
{
    m_downloadsPage->setProgress(percent, stage, speed, done, total);
    m_sidebar->setDownloadProgress(percent, stage, speed, done, total);
}

void MainWindow::onDownloadDone(bool ok, const QString& message)
{
    m_activeDownload = nullptr;
    m_sidebar->hideDownload();
    if (ok) {
        m_downloadsPage->complete(message);
        QTimer::singleShot(1000, this, &MainWindow::loadGames);
    } else {
        m_downloadsPage->failed(message);
    }
}

void MainWindow::cancelDownload()
{
    if (m_activeDownload) {
        auto* worker = qobject_cast<DownloadWorker*>(m_activeDownload);
        if (worker) worker->cancel();
    }
}

void MainWindow::openSettings()
{
    SettingsDialog dlg(this);
    if (dlg.exec() == QDialog::Accepted) {
        QString size = dlg.selectedCardSize();
        Config::instance().setDisplaySize(size);
        Config::instance().setGamesDir(dlg.gamesDirText());
        Config::instance().setSteamAppcache(dlg.steamAppcacheText());
        Config::instance().save();
        auto [w, h] = resolveCardSize(size);
        m_cardW = w;
        m_cardH = h;
        m_grid->setCardSize(m_cardW, m_cardH);
        loadGames();
    }
}

void MainWindow::updateProton()
{
    m_downloadsPage->begin("Updating Proton...");
    showDownloads();

    spawnWorker(new ProtonUpdateWorker(this), [this](ProtonUpdateWorker* worker) {
        connect(worker, &ProtonUpdateWorker::message, this, [this](const QString& msg) {
            m_downloadsPage->setProgress(0, msg, 0, 0, 0);
        });
        connect(worker, &ProtonUpdateWorker::done, this, [this](bool ok, const QString& msg) {
            if (ok) m_downloadsPage->complete(msg);
            else m_downloadsPage->failed(msg);
        });
    });
}

void MainWindow::playTick()
{
    if (m_playtime->tick()) {
        m_recent->setGames(m_playtime->recentlyPlayed(m_games));
    }
}

void MainWindow::shutdownThreads()
{
    m_closed = true;
    m_playtime->flush();
    stopArtThread(3000);
}
