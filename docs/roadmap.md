# forager Roadmap

Everything on this list must be in place before forager is tagged `v1.0.0`
("stable"). Until then the project stays on the `0.x` line, where the public
API and features may change at any time.

| Feature | Status | Notes |
|---------|--------|-------|
| Full Steam library (all owned, not just installed) | Done | `ISteamApps/GetOwnedGames` via the Steam Web API key + SteamID; owned-but-uninstalled titles merged into the library (source-only `Game`, no path) and show an Install action |
| Downloading Steam games | Done | DepotDownloader driven by stored credentials; `DownloadWorker` streams `DownloadProgress` into the Downloads page |
| Buying Steam games | Done | `QWebEngineView` wrapper of the real Steam store, recolored with an injected stylesheet; gracefully reports the missing `PySide6-WebEngine` dependency when absent |
| Torrenting | Done | `libtorrent` integration as a generic downloader; legality of torrented content is the user's responsibility |
| Steam achievements (view/display) | Done | Local `achievements.vdf` parse + `GetPlayerAchievements` Web API, surfaced in the `GamePage` panel |
| Epic Games support | Done | Legendary (open-source EGS CLI) backend: auth, list owned, install, launch |
| GOG support | Done | GOG unofficial web API for owned offline installers; token stored in the keyring |

## Build order

1. Steam account (done: QR + username/password sign-in, persistent refresh-token session)
2. Full Steam library (done)
3. Downloading Steam games (done)
4. Store webview for buying (done)
5. Epic Games (done)
6. GOG (done)
7. Steam achievements (done)
8. Torrenting (done)

All roadmap items are implemented. The remaining work before `v1.0.0` is
polish, real-credential/network testing, and packaging (the store webview
requires the optional `PySide6-WebEngine` dependency).
