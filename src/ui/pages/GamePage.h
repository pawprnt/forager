#pragma once

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QScrollArea>
#include <QFrame>
#include <QListWidget>
#include <QPixmap>
#include <QTimer>
#include <QThread>
#include "core/Game.h"

class Banner;

class GamePage : public QWidget {
    Q_OBJECT

public:
    explicit GamePage(QWidget* parent = nullptr);

    void setGame(const Game& game);
    void setRunning(bool running);
    void setHero(const QPixmap& pix);

signals:
    void play(const Game& game);
    void stop(const Game& game);
    void install(const Game& game);
    void backRequested();

private slots:
    void onPlayClicked();

private:
    QWidget* buildBannerOverlay();
    QFrame* makeBox(const QString& title, QVBoxLayout** outLay = nullptr, QLabel** outHeader = nullptr);
    QFrame* buildInfoBox();
    QFrame* buildAchievementsBox();
    void populateAchievements(const Game& game);
    void updatePlayButton();

    Game m_game;
    QPixmap m_logo;
    bool m_running = false;

    Banner* m_banner = nullptr;
    QLabel* m_title = nullptr;
    QLabel* m_sourceBadge = nullptr;
    QLabel* m_pathLabel = nullptr;
    QPushButton* m_playBtn = nullptr;
    QLabel* m_playIconLabel = nullptr;
    QLabel* m_playText = nullptr;
    QFrame* m_infoBox = nullptr;
    QLabel* m_infoSource = nullptr;
    QLabel* m_infoAppId = nullptr;
    QFrame* m_achFrame = nullptr;
    QLabel* m_achHeader = nullptr;
    QListWidget* m_achList = nullptr;
    QLabel* m_logoLabel = nullptr;
    QWidget* m_bannerOverlay = nullptr;
};
