#pragma once

#include <QThread>
#include <QObject>
#include <QPixmap>
#include <QMutex>
#include <QList>
#include <functional>
#include "core/Game.h"

class ScanWorker : public QThread {
    Q_OBJECT

public:
    explicit ScanWorker(QObject* parent = nullptr);

signals:
    void done(const QList<Game>& games);

protected:
    void run() override;
};

class ProtonUpdateWorker : public QThread {
    Q_OBJECT

public:
    explicit ProtonUpdateWorker(QObject* parent = nullptr);

signals:
    void message(const QString& text);
    void progress(double percent);
    void done(bool ok, const QString& message);

protected:
    void run() override;
};

class DownloadWorker : public QThread {
    Q_OBJECT

public:
    DownloadWorker(const QString& providerName, const QString& appId,
                   const QString& destination, QObject* parent = nullptr);

    void cancel();

signals:
    void progress(double percent, const QString& stage, double speed, double done, double total);
    void done(bool ok, const QString& message);

protected:
    void run() override;

private:
    QString m_provider;
    QString m_appId;
    QString m_dest;
    QMutex m_mutex;
    bool m_cancelled = false;
};

class ArtSignals : public QObject {
    Q_OBJECT

public:
    using QObject::QObject;

signals:
    void gridReady(const Game& game, const QByteArray& data);
    void iconReady(const Game& game, const QByteArray& data);
};

class HeroSignals : public QObject {
    Q_OBJECT

public:
    using QObject::QObject;

signals:
    void ready(const Game& game, const QByteArray& data);
};

namespace workers {
    void runArtJob(const QList<Game>& games, ArtSignals* artSignals, std::function<bool()> isStopped);
    void runHeroJob(const Game& game, HeroSignals* heroSignals, std::function<bool()> isStopped);
} // namespace workers
