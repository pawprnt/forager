#include "ui/MainWindow.h"
#include "ui/widgets/Sidebar.h"
#include "ui/widgets/TitleBar.h"
#include "core/Config.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>
#include <QLabel>
#include <QStyle>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    Config::instance().load();
    setWindowTitle("forager");
    resize(1280, 720);

    setupUi();
    applyStyle();

    // Defer heavy loading until window is visible
    m_startupTimer = new QTimer(this);
    m_startupTimer->setSingleShot(true);
    connect(m_startupTimer, &QTimer::timeout, this, [this]() {
        // TODO: load games, start art workers
    });
    m_startupTimer->start(50);
}

void MainWindow::setupUi()
{
    auto* central = new QWidget(this);
    auto* layout = new QHBoxLayout(central);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    // Sidebar placeholder
    m_sidebar = new Sidebar(central);
    layout->addWidget(m_sidebar);

    // Main content area
    auto* contentLayout = new QVBoxLayout();
    contentLayout->setContentsMargins(0, 0, 0, 0);
    contentLayout->setSpacing(0);

    m_titlebar = new TitleBar(central);
    contentLayout->addWidget(m_titlebar);

    m_pages = new QStackedWidget(central);
    // TODO: add pages (GameGrid, GamePage, Downloads, Store)
    auto* placeholder = new QLabel("forager");
    placeholder->setAlignment(Qt::AlignCenter);
    placeholder->setObjectName("placeholder");
    m_pages->addWidget(placeholder);
    contentLayout->addWidget(m_pages);

    layout->addLayout(contentLayout, 1);

    setCentralWidget(central);
}

void MainWindow::applyStyle()
{
    setStyleSheet(R"(
        QMainWindow {
            background-color: #0e1015;
        }
        #placeholder {
            color: #6b7280;
            font-size: 18px;
            font-family: "Be Vietnam Pro";
        }
    )");
}

void MainWindow::onSearchChanged(const QString& query)
{
    Q_UNUSED(query);
    // TODO: filter game grid
}
