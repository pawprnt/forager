# roadmap

## v0.5.x — C++ rewrite

the python/pyside6 codebase has been rewritten in c++17 with qt6 widgets.

### done

| feature | status |
|---------|--------|
| cmake + justfile build system | done |
| core data model (game, config, paths) | done |
| library scanner (steam, minecraft, standalone) | done |
| game launcher (steam, minecraft, proton, native) | done |
| playtime tracking | done |
| ui foundation (theme, mainwindow, sidebar, titlebar) | done |
| game grid + game cards with hover animation | done |
| cover art pipeline (local, cache, cdn, sgdb, placeholder) | done |
| placeholder art generation | done |
| steam account auth (qr + password) | done |
| steamgriddb integration + keyring storage | done |
| settings dialog (library, proton, account tabs) | done |
| nix package (wrapqtappshook) | done |

### in progress

| feature | target |
|---------|--------|
| downloads page + progress | v0.6.0 |
| store webview (qwebengine) | v0.6.0 |
| game page detail view | v0.6.0 |
| epic games provider | v0.6.0 |
| gog provider | v0.6.0 |
| torrent provider | v0.6.0 |

### planned

| feature | target |
|---------|--------|
| proton management (steamcmd, prefix, updates) | v0.7.0 |
| tool updates (github releases) | v0.7.0 |
| gamepad navigation (libevdev) | v0.7.0 |
| controller nav widget | v0.7.0 |
| unit tests | v0.7.0 |
| flatpak + appimage packaging | v0.8.0 |
| custom themes / color schemes | v0.8.0 |
| minecraft instances & skins | v0.8.0 |
| itch.io store integration | v0.9.0 |
| stable v1.0.0 release | v1.0.0 |

## questioning

| idea | notes |
|------|-------|
| windows native port | qt6 is cross-platform, but forager uses libevdev (gamepad) and libsecret (keyring) which are linux-only. a windows port would need alternatives: sdl2 for gamepad, win32 credential manager or similar for secrets. feasible but significant effort. |
| cloud sync | steam cloud integration for saves — post-v1.0 |
| wayland-native gamepad | evdev works but raw wayland protocols may be better long-term |
