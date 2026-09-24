#include "ui/Workers.h"
#include "library/Scanner.h"
#include "artwork/Pipeline.h"
#include "artwork/PixmapUtils.h"
#include <QMutexLocker>
#include <QBuffer>
#include <algorithm>

// -- ScanWorker ------------------------------------------------------------

ScanWorker::ScanWorker(QObject* parent)
    : QThread(parent)
{
}

void ScanWorker::run() {
    std::vector<Game> vec = scanner::scanAll();
    QList<Game> games;
    games.reserve(vec.size());
    for (auto& g : vec) games.append(std::move(g));
    if (!isInterruptionRequested())
        emit done(games);
}

// -- ProtonUpdateWorker ----------------------------------------------------

ProtonUpdateWorker::ProtonUpdateWorker(QObject* parent)
    : QThread(parent)
{
}

void ProtonUpdateWorker::run() {
    emit done(false, "Proton update not yet implemented");
}

// -- DownloadWorker --------------------------------------------------------

DownloadWorker::DownloadWorker(const QString& providerName, const QString& appId,
                               const QString& destination, QObject* parent)
    : QThread(parent), m_provider(providerName), m_appId(appId), m_dest(destination)
{
}

void DownloadWorker::cancel() {
    QMutexLocker lock(&m_mutex);
    m_cancelled = true;
}

void DownloadWorker::run() {
    emit done(false, "Download not yet implemented");
}

// -- Art job functions -----------------------------------------------------

void workers::runArtJob(const QList<Game>& games, ArtSignals* artSignals, std::function<bool()> isStopped) {
    for (const auto& game : games) {
        if (isStopped()) return;
        QPixmap grid = art::loadGrid(game, true);
        if (!grid.isNull())
            emit artSignals->gridReady(game, pixmap::toJpeg(grid));
        if (isStopped()) return;
        QPixmap icon = art::loadIcon(game, true);
        if (!icon.isNull()) {
            QByteArray png;
            QBuffer buf(&png);
            buf.open(QIODevice::WriteOnly);
            if (icon.save(&buf, "PNG"))
                emit artSignals->iconReady(game, png);
        }
        if (isStopped()) return;
    }
}

void workers::runHeroJob(const Game& game, HeroSignals* heroSignals, std::function<bool()> isStopped) {
    if (isStopped()) return;
    QPixmap pix = art::loadHero(game, true);
    if (!pix.isNull())
        emit heroSignals->ready(game, pixmap::toJpeg(pix));
}
