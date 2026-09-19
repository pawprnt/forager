# Forager C++ Rewrite Plan

## Overview

Rewrite the Python/PySide6 forager game launcher in C++17 with Qt6 Widgets.
Build system: CMake + Justfile. Same repo, new `cpp` branch.

**Goals:** drop Python runtime + PySide6 pip deps, eliminate GIL-bound paint jank,
cut startup time, native feel.

**Tradeoffs:** loss of Python hot-reload, harder for Python contributors.

---

## Dependency Map

| Python | C++ Equivalent | Notes |
|--------|---------------|-------|
| PySide6 | Qt6 (Widgets, Network, Concurrent, WebEngineWidgets) | Same framework, C++ API |
| evdev | libevdev | C library, same API surface |
| keyring | libsecret | D-Bus Secret Service API |
| Pillow | Qt6 QImage/QPixmap | Built-in, no extra dep |
| qrcode | qrencode | C library |
| urllib (stdlib) | QNetworkAccessManager | Qt networking |
| subprocess | QProcess | Qt process management |
| threading | QThread / QtConcurrent | Qt threading |
| json (stdlib) | QJsonDocument | Qt JSON |
| pathlib | std::filesystem | C++17 |

---

## Project Structure

```
forager/
├── CMakeLists.txt              # root cmake
├── justfile                    # build commands
├── src/
│   ├── main.cpp                # entry point
│   ├── app/
│   │   ├── Application.h/cpp   # QApplication subclass
│   │   └── Constants.h         # APP_NAME, VERSION
│   ├── core/
│   │   ├── Game.h/cpp          # Game dataclass equivalent
│   │   ├── Source.h            # Source enum
│   │   ├── Config.h/cpp        # Settings (JSON config)
│   │   ├── Paths.h/cpp         # filesystem path helpers
│   │   └── Controller.h/cpp    # gamepad polling (libevdev)
│   ├── library/
│   │   ├── Scanner.h/cpp       # game discovery
│   │   ├── Launcher.h/cpp      # game launch dispatch
│   │   ├── Playtime.h/cpp      # playtime tracking
│   │   └── Metadata.h/cpp      # search/filter helpers
│   ├── artwork/
│   │   ├── Pipeline.h/cpp      # art resolution cascade
│   │   ├── Cache.h/cpp         # disk cache management
│   │   ├── Placeholder.h/cpp   # generated fallback art
│   │   ├── PeIcons.h/cpp       # PE executable icon extractor
│   │   └── PixmapUtils.h/cpp   # scale, crop, bytes-to-pixmap
│   ├── providers/
│   │   ├── Provider.h/cpp      # abstract base + registry
│   │   ├── steam/
│   │   │   ├── SteamProvider.h/cpp
│   │   │   ├── SteamAuth.h/cpp       # QR/password auth via Web API
│   │   │   ├── SteamCredentials.h/cpp # libsecret storage
│   │   │   ├── SteamLibrary.h/cpp    # owned games list
│   │   │   ├── SteamDownloader.h/cpp # DepotDownloader wrapper
│   │   │   ├── SteamAchievements.h/cpp # VDF parser
│   │   │   └── SteamAppId.h/cpp      # app ID resolution
│   │   ├── epic/
│   │   │   └── EpicProvider.h/cpp    # Legendary CLI wrapper
│   │   ├── gog/
│   │   │   └── GogProvider.h/cpp     # GOG web API
│   │   └── torrent/
│   │       └── TorrentProvider.h/cpp  # libtorrent wrapper
│   ├── services/
│   │   ├── SteamGridDB.h/cpp   # SGDB API client
│   │   └── IconProvider.h/cpp  # game icon resolution
│   ├── compatibility/
│   │   └── Proton.h/cpp        # proton runtime management
│   ├── updates/
│   │   └── ToolUpdates.h/cpp   # GitHub releases updater
│   ├── ui/
│   │   ├── MainWindow.h/cpp
│   │   ├── Theme.h/cpp         # palette + QSS
│   │   ├── Style.h/cpp         # QSS builder functions
│   │   ├── Icons.h/cpp         # SVG icon loader
│   │   ├── Fonts.h/cpp         # font registration
│   │   ├── Workers.h/cpp       # QThread worker wrappers
│   │   ├── pages/
│   │   │   ├── GameGrid.h/cpp
│   │   │   ├── GamePage.h/cpp
│   │   │   ├── Downloads.h/cpp
│   │   │   └── Store.h/cpp
│   │   ├── widgets/
│   │   │   ├── GameCard.h/cpp
│   │   │   ├── Banner.h/cpp
│   │   │   ├── Sidebar.h/cpp
│   │   │   ├── TitleBar.h/cpp
│   │   │   ├── RecentRow.h/cpp
│   │   │   ├── ControllerNav.h/cpp
│   │   │   ├── LoadingSpinner.h/cpp
│   │   │   └── DownloadBox.h/cpp
│   │   └── dialogs/
│   │       ├── SettingsDialog.h/cpp
│   │       ├── SettingsTabs.h/cpp
│   │       ├── AccountTab.h/cpp
│   │       ├── SteamAuthDialog.h/cpp
│   │       └── SteamGridDBDialog.h/cpp
│   └── utils/
│       ├── Network.h/cpp       # QNetworkAccessManager helpers
│       ├── Subprocess.h/cpp    # QProcess helpers
│       ├── Filesystem.h/cpp    # ensure_dir, etc.
│       └── Download.h/cpp      # download with progress
├── resources/
│   ├── forager.qrc             # Qt resource file
│   ├── icons/                  # Iconoir SVGs
│   ├── fonts/                  # Be Vietnam Pro
│   └── pages/                  # CSS files for store webview
├── tests/
│   ├── CMakeLists.txt
│   └── ...
├── nix/
│   └── default.nix             # NixOS package (mkDerivation)
└── packaging/
    └── ...                     # appimage, etc.
```

---

## Justfile Targets

```just
# default
default: build

# configure cmake
configure:
    cmake -B build -DCMAKE_BUILD_TYPE=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS=ON

# build
build:
    cmake --build build -j$(nproc)

# build release
release:
    cmake -B build -DCMAKE_BUILD_TYPE=Release
    cmake --build build -j$(nproc)

# run
run: build
    ./build/forager

# clean
clean:
    rm -rf build/

# clean + rebuild
rebuild: clean configure build

# run tests
test: build
    cd build && ctest --output-on-failure

# install
install:
    cmake --install build

# format code
format:
    find src/ -name '*.cpp' -o -name '*.h' | xargs clang-format -i

# lint
lint:
    clang-tidy src/**/*.cpp -- -std=c++17 -Isrc/

# update compile_commands.json symlink
compiledb: configure
    ln -sf build/compile_commands.json .
```

---

## Implementation Phases

### Phase 1: Skeleton + Build System (~15h)
- CMakeLists.txt with Qt6, libevdev, libsecret, qrencode find_package
- justfile
- main.cpp + Application class
- Constants, Paths
- Empty MainWindow that shows
- Nix package (mkDerivation with Qt6)
- Git branch `cpp`

### Phase 2: Core Data Model (~20h)
- Game, Source, Config, Paths
- Provider base class + registry
- Settings JSON read/write (QJsonDocument)

### Phase 3: Library Logic (~25h)
- Scanner (directory walk, ACF parse, Minecraft detection)
- Launcher (QProcess dispatch to Steam/Minecraft/standalone)
- Playtime tracking (JSON persistence)
- Metadata search/filter

### Phase 4: UI Foundation (~40h)
- Theme + Style (QPalette, QSS)
- MainWindow layout (titlebar, sidebar, stacked pages)
- GameGrid with GameCard custom paint
- Sidebar with search
- TitleBar with tabs
- Fonts + Icons
- LoadingSpinner

### Phase 5: Art Pipeline (~25h)
- Art resolution cascade (local -> cache -> CDN -> SGDB -> placeholder)
- Placeholder art generation (QPainter)
- PE icon extractor (MZ/PE header parsing)
- Disk cache (QDir + checksum)
- PixmapUtils

### Phase 6: Providers (~50h)
- Steam: credentials (libsecret), auth (QR + password via QNetworkAccessManager), library, downloader (QProcess + DepotDownloader), achievements (VDF parse), AppId resolver
- Epic: Legendary CLI wrapper (QProcess)
- GOG: web API client (QNetworkAccessManager)
- Torrent: libtorrent wrapper

### Phase 7: Services (~15h)
- SteamGridDB API client
- IconProvider

### Phase 8: Compatibility + Updates (~15h)
- Proton management (steamcmd, symlinks, prefix)
- Tool updates (GitHub releases API)

### Phase 9: Dialogs + Polish (~30h)
- SettingsDialog (Library, Proton, Account tabs)
- SteamAuthDialog (QR display, password fallback)
- SteamGridDBTokenDialog (QWebEngineView)
- DownloadsPage
- StorePage (QWebEngineView with injected CSS)
- GamePage detail view
- Gamepad navigation (libevdev → action mapping)
- ControllerNav

### Phase 10: Packaging + Testing (~20h)
- Nix package finalization
- AppImage / Flatpak
- Unit tests for core, library, artwork
- Integration test for providers

**Total estimate: ~255h**

---

## Threading Model (C++ equivalent)

| Python Pattern | C++ Equivalent |
|---------------|---------------|
| QThread subclass | QThread + moveToThread |
| threading.Thread | QtConcurrent::run or QThread |
| threading.Event | QAtomicInt / std::atomic<bool> + QWaitCondition |
| threading.Lock | QMutex |
| Signal/Signal | Qt signals/slots |
| QObject bridge pattern | Direct signal emission from worker object |

---

## Key Decisions

1. **JSON**: QJsonDocument (built-in, no nlohmann dependency)
2. **Network**: QNetworkAccessManager throughout
3. **Image processing**: QPainter + QImage (replaces Pillow entirely)
4. **SVG icons**: QSvgRenderer (built-in)
5. **Fonts**: QFontDatabase::addApplicationFont
6. **Keyring**: libsecret (SecretPassword API)
7. **Evdev**: libevdev C API wrapped in a QThread
8. **QR codes**: qrencode → QImage
9. **PE parsing**: custom std::ifstream parser
10. **CSS injection**: QWebEnginePage::runJavaScript + QWebEngineProfile
