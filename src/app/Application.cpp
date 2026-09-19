#include "app/Application.h"
#include "app/Constants.h"
#include "ui/MainWindow.h"

#include <QFontDatabase>
#include <QStyleFactory>

Application::Application(int& argc, char** argv)
    : QApplication(argc, argv)
{
    setApplicationName(constants::APP_NAME);
    setOrganizationName(constants::ORGANIZATION);

    registerFonts();
    setupTheme();

    m_window = std::make_unique<MainWindow>();
    m_window->show();
}

Application::~Application() = default;

void Application::registerFonts()
{
    // TODO: register bundled Be Vietnam Pro fonts
}

void Application::setupTheme()
{
    // TODO: apply Fusion style + SpaceTheme palette
    setStyle(QStyleFactory::create("Fusion"));
}
