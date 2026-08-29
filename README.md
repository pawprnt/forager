# forager

a chill game launcher for your local library.
steam-like vibes, space theme aesthetics, no subscription fees.

## features

- **library view** — steam-style grid of cover tiles with a searchable sidebar
- **space theme ui** — dark, layered, rounded look inspired by [SpaceTheme](https://github.com/SpaceTheme/Steam)
- **gamepad support** — navigate and launch with a controller (via `evdev`)
- **cover art** — pulls art from local steam files, the steam CDN, and steamgriddb
- **steam account** — sign in with the steam mobile app (QR code) or username/password
- **full steam library** — shows all owned games, not just installed ones
- **steam downloads** — download and install steam games directly
- **store** — browse and buy games from the steam store in-app
- **epic games** — epic games support via legendary
- **gog** — gog support for offline installers
- **torrents** — torrent downloads via libtorrent
- **steam achievements** — view your achievements on the game page
- **proton** — runs windows `.exe` games through a shared proton prefix
- **tool updates** — keeps bundled tools up to date with live progress

## roadmap

all roadmap items are implemented. remaining work before `v1.0.0` is polish, testing, and packaging.

| feature | status |
|---------|--------|
| full steam library | done |
| steam downloads | done |
| store webview | done |
| epic games | done |
| gog | done |
| steam achievements | done |
| torrenting | done |

## install

### from flatpak

```
flatpak remote-add --user pawprnt https://pawprnt.github.io/flatpak-repo/repo
flatpak install --user pawprnt io.github.pawprnt.forager
flatpak run io.github.pawprnt.forager
```

### from the aur (arch linux)

```
paru -S forager
```

### from source

```
git clone https://github.com/pawprnt/forager.git
cd forager
python -m venv .venv
.venv/bin/pip install -e .
.venv/bin/forager
```

### system-wide (arch linux)

```
sudo pacman -S python-pyside6 python-evdev python-keyring python-pillow
git clone https://github.com/pawprnt/forager.git
cd forager
sudo python -m pip install --break-system-packages --no-deps .
forager
```

## configuration

settings are stored in `~/.config/forager/settings.json`.
cover art caches live in `~/.cache/forager/`.

environment overrides:

- `FORAGER_CONFIG_DIR` — config directory (default `~/.config/forager`)
- `FORAGER_CACHE_DIR` — cache directory (default `~/.cache/forager`)
- `STEAMGRIDDB_API_KEY` — steamgriddb token fallback

## library layout

your game library folder should look like:

```
~/Games/
├── steam/
│   └── steamapps/
├── minecraft/
└── drm-free/
    ├── standalone/
    │   └── <engine>/
    │       └── <game>/
    └── series/
        └── <engine>/
            └── <series>/
                └── <game>/
```

games are detected by an executable or `Game.ini` in the folder.

## license

AGPL-3.0

bundled third-party assets carry their own licenses:
- [Iconoir](https://iconoir.com) (MIT) — UI icons
- [VT323](https://github.com/google/fonts/tree/main/ofl/vt323) (SIL OFL 1.1) — placeholder art font
- [FluentSystemIcons](https://github.com/microsoft/fluentui-system-icons) (MIT) — store webview icons
- [SpaceTheme](https://github.com/SpaceTheme/Steam) (MIT) — store webview styling
