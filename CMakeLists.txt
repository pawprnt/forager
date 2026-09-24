cmake_minimum_required(VERSION 3.20)
project(forager VERSION 0.5.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Network Concurrent WebEngineWidgets Svg Test)
find_package(PkgConfig REQUIRED)

qt_standard_project_setup(REQUIRES 6.6)

pkg_check_modules(LIBEVDEV REQUIRED libevdev)
pkg_check_modules(LIBSECRET REQUIRED libsecret-1)
pkg_check_modules(QRENCODE REQUIRED libqrencode)

set(FORAGER_LIB_SOURCES
    src/app/Application.cpp
    src/core/Paths.cpp
    src/core/Config.cpp
    src/core/Game.cpp
    src/providers/Provider.cpp
    src/providers/steam/SteamCredentials.cpp
    src/providers/steam/SteamProvider.cpp
    src/providers/steam/SteamLibrary.cpp
    src/providers/steam/SteamDownloader.cpp
    src/providers/steam/SteamAchievements.cpp
    src/providers/steam/SteamAppId.cpp
    src/providers/epic/EpicProvider.cpp
    src/providers/gog/GogProvider.cpp
    src/providers/torrent/TorrentProvider.cpp
    src/library/Scanner.cpp
    src/library/Launcher.cpp
    src/library/Playtime.cpp
    src/library/Metadata.cpp
    src/minecraft/VersionJson.cpp
    src/minecraft/Classpath.cpp
    src/minecraft/Instance.cpp
    src/minecraft/JavaDetect.cpp
    src/minecraft/AuthSession.cpp
    src/minecraft/Assets.cpp
    src/minecraft/Launch.cpp
    src/artwork/Cache.cpp
    src/artwork/Pipeline.cpp
    src/artwork/PipelineCdn.cpp
    src/artwork/PipelineSgdb.cpp
    src/artwork/PipelineSteam.cpp
    src/artwork/PixmapUtils.cpp
    src/artwork/Placeholder.cpp
    src/services/SteamGridDB.cpp
    src/services/IconProvider.cpp
    src/compatibility/Proton.cpp
    src/updates/ToolUpdates.cpp
    src/utils/Network.cpp
    src/utils/Subprocess.cpp
    src/utils/Filesystem.cpp
    src/utils/Json.cpp
    src/utils/Acf.cpp
    src/utils/Secrets.cpp
    src/ui/Theme.cpp
    src/ui/Style.cpp
    src/ui/Fonts.cpp
    src/ui/Icons.cpp
    src/ui/Workers.cpp
    src/ui/MainWindow.cpp
    src/ui/MainWindowPages.cpp
    src/ui/MainWindowSearch.cpp
    src/ui/MainWindowArt.cpp
    src/ui/widgets/Sidebar.cpp
    src/ui/widgets/TitleBar.cpp
    src/ui/widgets/GameCard.cpp
    src/ui/widgets/LoadingSpinner.cpp
    src/ui/widgets/Banner.cpp
    src/ui/widgets/RecentRow.cpp
    src/ui/widgets/DownloadBox.cpp
    src/ui/widgets/ControllerNav.cpp
    src/ui/pages/GameGrid.cpp
    src/ui/pages/GamePage.cpp
    src/ui/pages/Downloads.cpp
    src/ui/pages/DownloadsProgress.cpp
    src/ui/pages/Store.cpp
    src/ui/dialogs/SettingsDialog.cpp
    src/ui/dialogs/SettingsTabs.cpp
    src/ui/dialogs/SteamAuthDialog.cpp
    src/ui/dialogs/SteamAuthQr.cpp
    src/ui/dialogs/SteamAuthPassword.cpp
    src/ui/dialogs/SteamGridDBDialog.cpp
)
set(FORAGER_HEADERS
    src/app/Application.h
    src/app/Constants.h
    src/core/Paths.h
    src/core/Config.h
    src/core/Game.h
    src/core/Source.h
    src/providers/Provider.h
    src/providers/steam/SteamCredentials.h
    src/providers/steam/SteamProvider.h
    src/providers/steam/SteamLibrary.h
    src/providers/steam/SteamDownloader.h
    src/providers/steam/SteamAchievements.h
    src/providers/steam/SteamAppId.h
    src/providers/epic/EpicProvider.h
    src/providers/gog/GogProvider.h
    src/providers/torrent/TorrentProvider.h
    src/library/Scanner.h
    src/library/Launcher.h
    src/library/Playtime.h
    src/library/Metadata.h
    src/minecraft/VersionJson.h
    src/minecraft/Classpath.h
    src/minecraft/Instance.h
    src/minecraft/JavaDetect.h
    src/minecraft/AuthSession.h
    src/minecraft/Assets.h
    src/minecraft/Launch.h
    src/artwork/Cache.h
    src/artwork/Pipeline.h
    src/artwork/PipelineInternal.h
    src/artwork/PixmapUtils.h
    src/artwork/Placeholder.h
    src/services/SteamGridDB.h
    src/services/IconProvider.h
    src/compatibility/Proton.h
    src/updates/ToolUpdates.h
    src/utils/Network.h
    src/utils/Subprocess.h
    src/utils/Filesystem.h
    src/utils/Json.h
    src/utils/Acf.h
    src/utils/Secrets.h
    src/utils/Format.h
    src/ui/Theme.h
    src/ui/Style.h
    src/ui/Fonts.h
    src/ui/Icons.h
    src/ui/Workers.h
    src/ui/MainWindow.h
    src/ui/widgets/Sidebar.h
    src/ui/widgets/TitleBar.h
    src/ui/widgets/GameCard.h
    src/ui/widgets/LoadingSpinner.h
    src/ui/widgets/Banner.h
    src/ui/widgets/RecentRow.h
    src/ui/widgets/DownloadBox.h
    src/ui/widgets/ControllerNav.h
    src/ui/pages/GameGrid.h
    src/ui/pages/GamePage.h
    src/ui/pages/Downloads.h
    src/ui/pages/DownloadsDetail.h
    src/ui/pages/Store.h
    src/ui/dialogs/SettingsDialog.h
    src/ui/dialogs/SteamAuthDialog.h
    src/ui/dialogs/SteamGridDBDialog.h
)

add_library(forager_lib STATIC ${FORAGER_LIB_SOURCES} ${FORAGER_HEADERS})

target_include_directories(forager_lib PUBLIC
    ${CMAKE_SOURCE_DIR}/src
    ${LIBEVDEV_INCLUDE_DIRS}
    ${LIBSECRET_INCLUDE_DIRS}
    ${QRENCODE_INCLUDE_DIRS}
)

target_link_libraries(forager_lib PUBLIC
    Qt6::Widgets
    Qt6::Network
    Qt6::Concurrent
    Qt6::WebEngineWidgets
    Qt6::Svg
    ${LIBEVDEV_LIBRARIES}
    ${LIBSECRET_LIBRARIES}
    ${QRENCODE_LIBRARIES}
)

target_compile_options(forager_lib PRIVATE
    ${LIBEVDEV_CFLAGS_OTHER}
    ${LIBSECRET_CFLAGS_OTHER}
    ${QRENCODE_CFLAGS_OTHER}
    -Wall -Wextra -Wpedantic
)

add_executable(forager src/main.cpp)
target_link_libraries(forager PRIVATE forager_lib)

install(TARGETS forager RUNTIME DESTINATION bin)

enable_testing()

set(FORAGER_TESTS
    tests/library/tst_game.cpp
    tests/library/tst_scanner.cpp
    tests/library/tst_launcher.cpp
    tests/library/tst_playtime.cpp
    tests/artwork/tst_placeholder.cpp
    tests/providers/tst_steamappid.cpp
    tests/utils/tst_acf.cpp
    tests/utils/tst_json.cpp
    tests/utils/tst_format.cpp
    tests/ui/tst_settings.cpp
    tests/ui/tst_store.cpp
    tests/ui/tst_downloads.cpp
    tests/ui/tst_icons.cpp
    tests/ui/tst_mainwindow.cpp
    tests/updates/tst_toolupdates.cpp
    tests/minecraft/tst_versionjson.cpp
    tests/minecraft/tst_classpath.cpp
    tests/minecraft/tst_instance.cpp
    tests/minecraft/tst_javadetect.cpp
    tests/minecraft/tst_authsession.cpp
    tests/minecraft/tst_assets.cpp
)

foreach(test_src IN LISTS FORAGER_TESTS)
    get_filename_component(test_name ${test_src} NAME_WE)
    add_executable(${test_name} ${test_src})
    target_link_libraries(${test_name} PRIVATE forager_lib Qt6::Test)
    target_compile_definitions(${test_name} PRIVATE SOURCE_DIR="${CMAKE_SOURCE_DIR}")
    add_test(NAME ${test_name} COMMAND ${test_name})
    set_tests_properties(${test_name} PROPERTIES
        ENVIRONMENT "QT_QPA_PLATFORM=offscreen;QTWEBENGINE_DISABLE_SANDBOX=1;DBUS_SESSION_BUS_ADDRESS="
        TIMEOUT 30
    )
endforeach()
