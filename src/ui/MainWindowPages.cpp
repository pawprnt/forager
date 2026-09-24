#include "ui/MainWindow.h"
#include "ui/widgets/Sidebar.h"
#include "ui/widgets/TitleBar.h"
#include "ui/widgets/RecentRow.h"
#include "ui/widgets/LoadingSpinner.h"
#include "ui/pages/GameGrid.h"
#include "ui/pages/GamePage.h"
#include "ui/pages/Downloads.h"
#include "ui/pages/Store.h"
#include "ui/Style.h"
#include "ui/Theme.h"
#include "library/Playtime.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>

using namespace theme;
using namespace theme::C;

void MainWindow::setupUi()
{
    auto* central = new QWidget(this);
    auto* mainLayout = style::hbox(central);

    auto* right = new QWidget(central);
    style::transparent(right);
    auto* rightLayout = style::vbox(right);

    m_titlebar = new TitleBar(right);
    rightLayout->addWidget(m_titlebar);

    m_content = new QStackedWidget(right);

    m_home = new QFrame();
    m_home->setObjectName("HomePage");
    style::transparent(m_home);
    auto* homeLayout = new QVBoxLayout(m_home);
    homeLayout->setContentsMargins(24, 18, 24, 18);
    homeLayout->setSpacing(16);

    auto* recentPanel = new QFrame();
    style::panel(recentPanel, 2);
    auto* recentLayout = new QVBoxLayout(recentPanel);
    recentLayout->setContentsMargins(16, 14, 16, 14);
    recentLayout->setSpacing(0);
    m_recent = new RecentRow(m_cardW > 165 ? 165 : m_cardW, m_cardH > 248 ? 248 : m_cardH);
    recentLayout->addWidget(m_recent);
    homeLayout->addWidget(recentPanel);

    auto* gridPanel = new QFrame();
    gridPanel->setObjectName("gridPanel");
    style::panel(gridPanel, 2);
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

    connect(m_recent, &RecentRow::gameClicked, this, &MainWindow::openGame);
}

void MainWindow::showPage(QWidget* page, bool back, const QString& tab)
{
    m_content->setCurrentWidget(page);
    m_titlebar->setBackEnabled(back);
    m_titlebar->setActiveTab(tab);
}

void MainWindow::showHome()
{
    showPage(m_home, false, "library");
}

void MainWindow::showStore()
{
    showPage(m_storePage, true, "store");
}

void MainWindow::showDownloads()
{
    showPage(m_downloadsPage, true, "library");
}

void MainWindow::openGame(const Game& game)
{
    if (game.name().isEmpty()) return;
    m_gamepage->setGame(game);
    m_gamepage->setRunning(m_playtime->isRunning(game));
    showPage(m_gamepage, true, "library");
    loadHeroAsync(game);
}
