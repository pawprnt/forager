#include "app/Application.h"
#include "app/Constants.h"
#include "ui/Fonts.h"
#include "ui/Theme.h"
#include "ui/MainWindow.h"

Application::Application(int& argc, char** argv)
    : QApplication(argc, argv)
{
    setApplicationName(constants::APP_NAME);
    setOrganizationName(constants::ORGANIZATION);

    fonts::registerFonts();
    theme::applyTheme(this);

    m_window = std::make_unique<MainWindow>();
    m_window->show();
}

Application::~Application() = default;
