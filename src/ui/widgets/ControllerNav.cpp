#include "ui/widgets/ControllerNav.h"
#include "core/Game.h"
#include <QMutexLocker>
#include <linux/input.h>
#include <fcntl.h>
#include <unistd.h>
#include <libevdev/libevdev.h>

// -- ControllerNav ---------------------------------------------------------

ControllerNav::ControllerNav(QObject* parent)
    : QThread(parent)
{
}

ControllerNav::~ControllerNav() {
    stop();
    wait(2000);
}

void ControllerNav::start() {
    QMutexLocker lock(&m_mutex);
    m_running = true;
    QThread::start();
}

void ControllerNav::stop() {
    QMutexLocker lock(&m_mutex);
    m_running = false;
    if (m_fd >= 0) {
        ::close(m_fd);
        m_fd = -1;
    }
}

void ControllerNav::run() {
    pollLoop();
}

void ControllerNav::pollLoop() {
    const char* devPath = nullptr;
    for (int i = 0; i < 32; ++i) {
        QString path = QStringLiteral("/dev/input/event%1").arg(i);
        int fd = ::open(path.toUtf8().constData(), O_RDONLY | O_NONBLOCK);
        if (fd < 0) continue;

        libevdev* dev = nullptr;
        if (libevdev_new_from_fd(fd, &dev) < 0) {
            ::close(fd);
            continue;
        }

        const char* name = libevdev_get_name(dev);
        if (name && (strstr(name, "Xbox") || strstr(name, "PlayStation") ||
                     strstr(name, "Nintendo") || strstr(name, "Controller") ||
                     strstr(name, "Gamepad"))) {
            QMutexLocker lock(&m_mutex);
            m_fd = fd;
            m_dev = dev;
            devPath = strdup(path.toUtf8().constData());
            break;
        }
        libevdev_free(dev);
        ::close(fd);
    }

    if (m_fd < 0) {
        emit connected(false);
        return;
    }

    emit connected(true);

    while (true) {
        QMutexLocker lock(&m_mutex);
        if (!m_running) break;
        lock.unlock();

        struct input_event ev;
        int rc = libevdev_next_event(m_dev, LIBEVDEV_READ_FLAG_NORMAL, &ev);
        if (rc == -EAGAIN || rc == -EINTR) {
            usleep(10000);
            continue;
        }
        if (rc != 0) break;

        if (ev.type == EV_KEY) {
            QString name;
            bool pressed = ev.value != 0;

            switch (ev.code) {
            case BTN_SOUTH: name = "a"; break;
            case BTN_EAST: name = "b"; break;
            case BTN_NORTH: name = "y"; break;
            case BTN_WEST: name = "x"; break;
            case BTN_START: name = "start"; break;
            case BTN_SELECT: name = "select"; break;
            case BTN_TL: name = "lb"; break;
            case BTN_TR: name = "rb"; break;
            default: continue;
            }
            emit button(name, pressed);
        }

        if (ev.type == EV_ABS) {
            bool pressed = ev.value != 0;
            QString dir;
            switch (ev.code) {
            case ABS_HAT0X:
                dir = ev.value < 0 ? "left" : ev.value > 0 ? "right" : "";
                break;
            case ABS_HAT0Y:
                dir = ev.value < 0 ? "up" : ev.value > 0 ? "down" : "";
                break;
            default: continue;
            }
            if (!dir.isEmpty())
                emit nav(dir, pressed);
        }
    }

    if (m_dev) {
        libevdev_free(m_dev);
        m_dev = nullptr;
    }
    if (m_fd >= 0) {
        ::close(m_fd);
        m_fd = -1;
    }
    free(const_cast<char*>(devPath));
    emit connected(false);
}

// -- GamepadNavigation -----------------------------------------------------

GamepadNavigation::GamepadNavigation(ControllerNav* controller, Callbacks cb, QObject* parent)
    : QObject(parent), m_controller(controller), m_cb(std::move(cb))
{
    connect(controller, &ControllerNav::connected, this, &GamepadNavigation::onConnected);
    connect(controller, &ControllerNav::button, this, &GamepadNavigation::onButton);
    connect(controller, &ControllerNav::nav, this, &GamepadNavigation::onNav);
    controller->start();
}

void GamepadNavigation::shutdown() {
    m_controller->stop();
    m_controller->wait(2000);
}

void GamepadNavigation::onConnected(bool connected) {
    m_connected = connected;
    refreshHint();
}

void GamepadNavigation::onButton(const QString& name, bool pressed) {
    if (!pressed) return;
    if (name == "a") {
        activate();
    } else if (name == "b") {
        m_cb.showHome();
    } else if (name == "start") {
        if (m_cb.isOnHome()) {
            Game game = m_cb.focusedGame();
            if (!game.name().isEmpty())
                m_cb.launchGame(game);
        }
    }
}

void GamepadNavigation::onNav(const QString& direction, bool pressed) {
    if (!pressed) return;
    if (direction == "left") {
        if (m_cb.isOnGamepage()) {
            m_cb.showHome();
        } else {
            m_cb.moveFocus(-1);
        }
    } else if (direction == "right") {
        m_cb.moveFocus(+1);
    } else if (direction == "up") {
        int cols = m_cb.columnCount();
        if (cols > 0) m_cb.moveFocus(-cols);
    } else if (direction == "down") {
        int cols = m_cb.columnCount();
        if (cols > 0) m_cb.moveFocus(+cols);
    }
    refreshHint();
}

void GamepadNavigation::activate() {
    if (m_cb.isOnHome()) {
        Game game = m_cb.focusedGame();
        if (!game.name().isEmpty())
            m_cb.openGame(game);
    } else if (m_cb.isOnGamepage()) {
        Game game = m_cb.gamepageGame();
        if (!game.name().isEmpty())
            m_cb.launchGame(game);
    }
}

void GamepadNavigation::refreshHint() {
    if (!m_connected) {
        m_cb.setHint("");
        return;
    }
    if (m_cb.isOnHome()) {
        m_cb.setHint("Gamepad \u00b7 A: Open  Start: Launch  B: Home");
    } else {
        m_cb.setHint("Gamepad \u00b7 A: Play  B: Back");
    }
}
