# roadmap

the python/pyside6 codebase has been rewritten in c++17 with qt6 widgets.

## done

| feature | status |
|---------|--------|
| cmake + justfile build system | done |
| core data model (game, config, paths) | done |
| library scanner (steam, minecraft, standalone) | done |
| game launcher | done |
| playtime tracking | done |
| ui (theme, mainwindow, sidebar, game cards) | done |
| cover art pipeline | done |
| steam account auth | done |
| steamgriddb + keyring storage | done |
| settings dialog | done |
| nix package | done |

## in progress

| feature | target |
|---------|--------|
| downloads page | v0.6.0 |
| store webview | v0.6.0 |
| game page detail view | v0.6.0 |
| epic/gog/torrent providers | v0.6.0 |

## planned

| feature | target |
|---------|--------|
| proton management | v0.7.0 |
| gamepad navigation | v0.7.0 |
| unit tests | v0.7.0 |
| flatpak + appimage | v0.8.0 |
| custom themes | v0.8.0 |
| stable v1.0.0 | v1.0.0 |

## questioning

| idea | notes |
|------|-------|
| windows port | qt6 is cross-platform, but libevdev + libsecret are linux-only. feasible with alternatives. |
| plugin system | third-party providers post-v1.0 |
| cloud sync | steam cloud — post-v1.0 |
