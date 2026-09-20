#include "ui/MainWindow.h"
#include "ui/widgets/Sidebar.h"
#include "ui/widgets/TitleBar.h"
#include "ui/widgets/GameCard.h"
#include "ui/widgets/RecentRow.h"
#include "ui/widgets/LoadingSpinner.h"
#include "ui/widgets/ControllerNav.h"
#include "ui/pages/GameGrid.h"
#include "ui/pages/GamePage.h"
#include "ui/pages/Downloads.h"
#include "ui/pages/Store.h"
#include "ui/Workers.h"
#include "ui/Theme.h"
#include "ui/Icons.h"
#include "ui/dialogs/SettingsDialog.h"
#include "core/Config.h"
#include "library/Scanner.h"
#include "library/Launcher.h"
#include "library/Playtime.h"
#include "library/Metadata.h"
#include "artwork/Pipeline.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>
#include <QMessageBox>
#include <QApplication>
#include <QSet>

using namespace theme;
using namespace theme::C;

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    Config::instance().load();
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
    m_closed = true;
    m_playtime->flush();
    if (m_artThread && m_artThread->isRunning()) {
        m_artThread->requestInterruption();
        m_artThread->quit();
        m_artThread->wait(3000);
    }
}

void MainWindow::setupUi()
{
    auto* central = new QWidget(this);
    auto* mainLayout = new QHBoxLayout(central);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    auto* right = new QWidget(central);
    right->setStyleSheet("background: transparent;");
    auto* rightLayout = new QVBoxLayout(right);
    rightLayout->setContentsMargins(0, 0, 0, 0);
    rightLayout->setSpacing(0);

    m_titlebar = new TitleBar(right);
    rightLayout->addWidget(m_titlebar);

    m_content = new QStackedWidget(right);

    m_home = new QFrame();
    m_home->setObjectName("HomePage");
    m_home->setStyleSheet("background: transparent;");
    auto* homeLayout = new QVBoxLayout(m_home);
    homeLayout->setContentsMargins(24, 18, 24, 18);
    homeLayout->setSpacing(16);

    auto* recentPanel = new QFrame();
    recentPanel->setStyleSheet(QStringLiteral("QFrame { background-color: %1; border-radius: %2px; }").arg(COLOR_2).arg(RADIUS));
    auto* recentLayout = new QVBoxLayout(recentPanel);
    recentLayout->setContentsMargins(16, 14, 16, 14);
    recentLayout->setSpacing(0);
    m_recent = new RecentRow(m_cardW > 165 ? 165 : m_cardW, m_cardH > 248 ? 248 : m_cardH);
    recentLayout->addWidget(m_recent);
    homeLayout->addWidget(recentPanel);

    auto* gridPanel = new QFrame();
    gridPanel->setObjectName("gridPanel");
    gridPanel->setStyleSheet(QStringLiteral("QFrame { background-color: %1; border-radius: %2px; }").arg(COLOR_2).arg(RADIUS));
    auto* gridLayout = new QVBoxLayout(gridPanel);
    gridLayout->setContentsMargins(16, 14, 16, 16);
    gridLayout->setSpacing(0);
    m_grid = new GameGrid(m_cardW, m_cardH);
    gridLayout->addWidget(m_grid);
    homeLayout->addWidget(gridPanel, 1);

    m_content->addWidget(m_home);

    m_gamepage = new GamePage();
    m_content->addWidget(m_gamepage);

    m_downloadsPage = new DownloadsPage();
    m_content->addWidget(m_downloadsPage);

    m_storePage = new StorePage();
    m_content->addWidget(m_storePage);

    rightLayout->addWidget(m_content, 1);
    mainLayout->addWidget(right, 1);

    m_sidebar = new Sidebar(central);
    mainLayout->addWidget(m_sidebar);

    m_loading = new LoadingSpinner(this);
    m_loading->setFixedSize(48, 48);
    m_loading->hide();

    setCentralWidget(central);

    connect(m_titlebar, &TitleBar::settingsRequested, this, &MainWindow::openSettings);
    connect(m_titlebar, &TitleBar::updateProtonRequested, this, &MainWindow::updateProton);
    connect(m_titlebar, &TitleBar::backRequested, this, &MainWindow::showHome);
    connect(m_titlebar, &TitleBar::storeTabRequested, this, &MainWindow::showStore);
    connect(m_titlebar, &TitleBar::libraryTabRequested, this, &MainWindow::showHome);

    connect(m_sidebar, &Sidebar::gameSelected, this, &MainWindow::openGame);
    connect(m_sidebar, &Sidebar::searchChanged, this, &MainWindow::onSearchChanged);
    connect(m_sidebar, &Sidebar::downloadClicked, this, &MainWindow::showDownloads);

    connect(m_downloadsPage, &DownloadsPage::cancelRequested, this, &MainWindow::cancelDownload);
    connect(m_downloadsPage, &DownloadsPage::settingsRequested, this, &MainWindow::openSettings);

    connect(m_gamepage, &GamePage::play, this, &MainWindow::launchGame);
    connect(m_gamepage, &GamePage::stop, this, &MainWindow::stopGame);
    connect(m_gamepage, &GamePage::install, this, &MainWindow::installGame);
    connect(m_gamepage, &GamePage::backRequested, this, &MainWindow::showHome);

    connect(m_grid, &GameGrid::cardClicked, this, &MainWindow::openGame);
    connect(m_grid, &GameGrid::cardActivated, this, &MainWindow::launchGame);
    connect(m_grid, &GameGrid::layoutChanged, this, &MainWindow::updateGridPanel);

    connect(m_recent, &RecentRow::gameClicked, this, &MainWindow::openGame);
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

    auto* worker = new ScanWorker(this);
    int gen = m_scanGeneration;
    connect(worker, &ScanWorker::done, this, [this, gen](const QList<Game>& games) {
        if (gen != m_scanGeneration) return;
        onGamesScanned(games);
    });
    worker->start();
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

void MainWindow::startArtWorker()
{
    if (m_artThread && m_artThread->isRunning()) {
        m_artThread->requestInterruption();
        m_artThread->quit();
        m_artThread->wait(2000);
    }

    m_artSignals = new ArtSignals(this);

    connect(m_artSignals, &ArtSignals::gridReady, this, &MainWindow::onGridReady, Qt::QueuedConnection);
    connect(m_artSignals, &ArtSignals::iconReady, this, &MainWindow::onIconReady, Qt::QueuedConnection);

    m_artThread = QThread::create([this]() {
        workers::runArtJob(m_games, m_artSignals, [this]() {
            return QThread::currentThread()->isInterruptionRequested();
        });
    });
    connect(m_artThread, &QThread::finished, m_artSignals, &QObject::deleteLater);
    m_artThread->start();
}

void MainWindow::onGridReady(const Game& game, const QByteArray& data)
{
    if (data.isEmpty()) return;
    QPixmap pix;
    pix.loadFromData(data);
    if (pix.isNull()) return;
    m_grid->setCardArt(game, pix);
    m_recent->setCardArt(game, pix);
}

void MainWindow::onIconReady(const Game& game, const QByteArray& data)
{
    if (data.isEmpty()) return;
    QPixmap pix;
    pix.loadFromData(data);
    if (!pix.isNull())
        m_sidebar->setGameArt(game, pix);
}

void MainWindow::onHeroReady(const Game& game, const QByteArray& data)
{
    if (m_closed) return;
    if (m_gamepage->findChild<QWidget*>()) {
        QString id = game.identifier();
        if (!m_heroDone.contains(id)) return;
    }
    if (data.isEmpty()) return;
    QPixmap pix;
    pix.loadFromData(data);
    if (!pix.isNull())
        m_gamepage->setHero(pix);
}

void MainWindow::showHome()
{
    m_content->setCurrentWidget(m_home);
    m_titlebar->setBackEnabled(false);
    m_titlebar->setActiveTab("library");
}

void MainWindow::showStore()
{
    m_content->setCurrentWidget(m_storePage);
    m_titlebar->setBackEnabled(true);
    m_titlebar->setActiveTab("store");
}

void MainWindow::showDownloads()
{
    m_content->setCurrentWidget(m_downloadsPage);
    m_titlebar->setBackEnabled(true);
    m_titlebar->setActiveTab("library");
}

void MainWindow::openGame(const Game& game)
{
    if (game.name().isEmpty()) return;
    m_gamepage->setGame(game);
    m_gamepage->setRunning(m_playtime->isRunning(game));
    m_content->setCurrentWidget(m_gamepage);
    m_titlebar->setBackEnabled(true);
    m_titlebar->setActiveTab("library");
    loadHeroAsync(game);
}

void MainWindow::loadHeroAsync(const Game& game)
{
    QString id = game.identifier();
    m_heroDone.remove(id);
    m_heroDone.insert(id);

    auto* heroSignals = new HeroSignals(this);

    connect(heroSignals, &HeroSignals::ready, this, [this, id](const Game& g, const QByteArray& data) {
        if (g.identifier() == id)
            onHeroReady(g, data);
    }, Qt::QueuedConnection);

    auto* thread = QThread::create([game, heroSignals]() {
        workers::runHeroJob(game, heroSignals, []() { return false; });
    });
    connect(thread, &QThread::finished, heroSignals, &QObject::deleteLater);
    connect(thread, &QThread::finished, thread, &QObject::deleteLater);
    thread->start();
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
    updateGridPanel();
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
    auto* worker = new DownloadWorker("steam", game.appId(), dest, this);
    m_activeDownload = worker;

    connect(worker, &DownloadWorker::progress, this, &MainWindow::onDownloadProgress);
    connect(worker, &DownloadWorker::done, this, &MainWindow::onDownloadDone);
    worker->start();
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

void MainWindow::onSearchChanged(const QString& query)
{
    m_grid->setSearch(query);
    m_recent->setSearch(query);
}

void MainWindow::openSettings()
{
    SettingsDialog dlg(this);
    if (dlg.exec() == QDialog::Accepted) {
        QString size = dlg.selectedCardSize();
        Config::instance().setDisplaySize(size);
        Config::instance().save();
        auto [w, h] = resolveCardSize(size);
        m_cardW = w;
        m_cardH = h;
        m_grid->setCardSize(m_cardW, m_cardH);
    }
}

void MainWindow::updateProton()
{
    m_downloadsPage->begin("Updating Proton...");
    showDownloads();

    auto* worker = new ProtonUpdateWorker(this);
    connect(worker, &ProtonUpdateWorker::message, this, [this](const QString& msg) {
        m_downloadsPage->setProgress(0, msg, 0, 0, 0);
    });
    connect(worker, &ProtonUpdateWorker::done, this, [this](bool ok, const QString& msg) {
        if (ok) m_downloadsPage->complete(msg);
        else m_downloadsPage->failed(msg);
    });
    worker->start();
}

void MainWindow::playTick()
{
    if (m_playtime->tick()) {
        m_recent->setGames(m_playtime->recentlyPlayed(m_games));
    }

    Game currentGame = m_gamepage->findChild<QWidget*>() ? Game() : Game();
    if (m_content->currentWidget() == m_gamepage) {
    }
}

void MainWindow::updateGridPanel()
{
}

void MainWindow::shutdownThreads()
{
    m_closed = true;
    m_playtime->flush();
    if (m_artThread && m_artThread->isRunning()) {
        m_artThread->requestInterruption();
        m_artThread->quit();
        m_artThread->wait(3000);
    }
}
