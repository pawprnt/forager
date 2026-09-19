#pragma once

#include <QObject>
#include <QThread>
#include <QMutex>
#include <functional>
#include "core/Game.h"

struct libevdev;

class ControllerNav : public QThread {
    Q_OBJECT

public:
    explicit ControllerNav(QObject* parent = nullptr);
    ~ControllerNav() override;

    void start();
    void stop();

signals:
    void connected(bool isConnected);
    void button(const QString& name, bool pressed);
    void nav(const QString& direction, bool pressed);

protected:
    void run() override;

private:
    void pollLoop();

    int m_fd = -1;
    bool m_running = false;
    QMutex m_mutex;
    libevdev* m_dev = nullptr;
};

class GamepadNavigation : public QObject {
    Q_OBJECT

public:
    struct Callbacks {
        std::function<bool()> isOnHome;
        std::function<bool()> isOnGamepage;
        std::function<Game()> focusedGame;
        std::function<Game()> gamepageGame;
        std::function<void(Game)> openGame;
        std::function<void(Game)> launchGame;
        std::function<void()> showHome;
        std::function<void(int)> moveFocus;
        std::function<int()> columnCount;
        std::function<void(QString)> setHint;
    };

    explicit GamepadNavigation(ControllerNav* controller, Callbacks cb, QObject* parent = nullptr);

    void shutdown();

private slots:
    void onConnected(bool connected);
    void onButton(const QString& name, bool pressed);
    void onNav(const QString& direction, bool pressed);

private:
    void activate();
    void refreshHint();

    ControllerNav* m_controller;
    Callbacks m_cb;
    bool m_connected = false;
};
