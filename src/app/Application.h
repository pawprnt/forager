#pragma once

#include <QApplication>
#include <memory>

class MainWindow;

class Application : public QApplication {
    Q_OBJECT

public:
    explicit Application(int& argc, char** argv);
    ~Application() override;

private:
    void setupTheme();
    void registerFonts();

    std::unique_ptr<MainWindow> m_window;
};
