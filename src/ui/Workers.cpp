#include "ui/Workers.h"
#include "library/Scanner.h"
#include <QMutexLocker>
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
        if (!game.appId().isEmpty()) {
            QByteArray data;
            if (!data.isEmpty())
                emit artSignals->gridReady(game, data);
        }
        if (isStopped()) return;
    }
}

void workers::runHeroJob(const Game& game, HeroSignals* heroSignals, std::function<bool()> isStopped) {
    if (isStopped()) return;
    QByteArray data;
    if (!data.isEmpty())
        emit heroSignals->ready(game, data);
}
