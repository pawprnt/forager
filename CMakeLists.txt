cmake_minimum_required(VERSION 3.20)
project(forager VERSION 0.5.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Network Concurrent WebEngineWidgets Svg)
find_package(PkgConfig REQUIRED)

qt_standard_project_setup(REQUIRES 6.6)

pkg_check_modules(LIBEVDEV REQUIRED libevdev)
pkg_check_modules(LIBSECRET REQUIRED libsecret-1)
pkg_check_modules(QRENCODE REQUIRED libqrencode)

set(FORAGER_SOURCES
    src/main.cpp
    src/app/Application.cpp
    src/core/Paths.cpp
    src/core/Config.cpp
    src/core/Game.cpp
    src/providers/Provider.cpp
    src/library/Scanner.cpp
    src/library/Launcher.cpp
    src/library/Playtime.cpp
    src/library/Metadata.cpp
    src/ui/MainWindow.cpp
    src/ui/widgets/Sidebar.cpp
    src/ui/widgets/TitleBar.cpp
)

set(FORAGER_HEADERS
    src/app/Application.h
    src/app/Constants.h
    src/core/Paths.h
    src/core/Config.h
    src/core/Game.h
    src/core/Source.h
    src/providers/Provider.h
    src/library/Scanner.h
    src/library/Launcher.h
    src/library/Playtime.h
    src/library/Metadata.h
    src/ui/MainWindow.h
    src/ui/widgets/Sidebar.h
    src/ui/widgets/TitleBar.h
)

add_executable(forager ${FORAGER_SOURCES} ${FORAGER_HEADERS})

target_include_directories(forager PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${LIBEVDEV_INCLUDE_DIRS}
    ${LIBSECRET_INCLUDE_DIRS}
    ${QRENCODE_INCLUDE_DIRS}
)

target_link_libraries(forager PRIVATE
    Qt6::Widgets
    Qt6::Network
    Qt6::Concurrent
    Qt6::WebEngineWidgets
    Qt6::Svg
    ${LIBEVDEV_LIBRARIES}
    ${LIBSECRET_LIBRARIES}
    ${QRENCODE_LIBRARIES}
)

target_compile_options(forager PRIVATE
    ${LIBEVDEV_CFLAGS_OTHER}
    ${LIBSECRET_CFLAGS_OTHER}
    ${QRENCODE_CFLAGS_OTHER}
    -Wall -Wextra -Wpedantic
)

install(TARGETS forager RUNTIME DESTINATION bin)
