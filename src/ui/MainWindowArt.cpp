#include "ui/MainWindow.h"
#include "ui/widgets/Sidebar.h"
#include "ui/widgets/RecentRow.h"
#include "ui/pages/GameGrid.h"
#include "ui/pages/GamePage.h"
#include "ui/Workers.h"

void MainWindow::stopArtThread(int waitMs)
{
    if (m_artThread && m_artThread->isRunning()) {
        m_artThread->requestInterruption();
        m_artThread->quit();
        m_artThread->wait(waitMs);
    }
}

void MainWindow::startArtWorker()
{
    stopArtThread(2000);

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

void MainWindow::applyImage(const QByteArray& data, const std::function<void(const QPixmap&)>& sink)
{
    if (data.isEmpty()) return;
    QPixmap pix;
    pix.loadFromData(data);
    if (pix.isNull()) return;
    sink(pix);
}

void MainWindow::onGridReady(const Game& game, const QByteArray& data)
{
    applyImage(data, [this, &game](const QPixmap& pix) {
        m_grid->setCardArt(game, pix);
        m_recent->setCardArt(game, pix);
    });
}

void MainWindow::onIconReady(const Game& game, const QByteArray& data)
{
    applyImage(data, [this, &game](const QPixmap& pix) {
        m_sidebar->setGameArt(game, pix);
    });
}

void MainWindow::onHeroReady(const Game& game, const QByteArray& data)
{
    if (m_closed) return;
    if (m_gamepage->findChild<QWidget*>()) {
        QString id = game.identifier();
        if (!m_heroDone.contains(id)) return;
    }
    applyImage(data, [this](const QPixmap& pix) {
        m_gamepage->setHero(pix);
    });
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
