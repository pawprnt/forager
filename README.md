<p align="center">
  <img src="readme/forager.svg" width="128" alt="forager icon" />
</p>

<h1 align="center">forager</h1>

<p align="center">
  a chill game launcher for your local library.<br/>
  steam-like vibes, space theme aesthetics, no subscription fees.
</p>

<p align="center">
  <a href="#install"><img src="https://img.shields.io/badge/install-green" alt="install" /></a>
  <a href="#features"><img src="https://img.shields.io/badge/features-blue" alt="features" /></a>
  <a href="readme/roadmap.md"><img src="https://img.shields.io/badge/roadmap-purple" alt="roadmap" /></a>
  <a href="readme/contrib.md"><img src="https://img.shields.io/badge/contributing-blueviolet" alt="contributing" /></a>
  <a href="https://github.com/pawprnt/forager/blob/main/LICENSE"><img src="https://img.shields.io/badge/license-AGPL--3.0-orange" alt="license" /></a>
</p>

<p align="center">
  <img src="https://img.shields.io/badge/platform-linux%20%7C%20windows%20(proton)-lightgrey" alt="platform" />
  <img src="https://img.shields.io/badge/python-3.10+-3776ab" alt="python" />
  <img src="https://img.shields.io/badge/Qt-6-41cd52" alt="qt" />
  <a href="https://github.com/pawprnt/forager/actions"><img src="https://img.shields.io/github/actions/workflow/status/pawprnt/forager/nix.yml?branch=main&label=nix%20build" alt="nix build" /></a>
</p>

---

## table of contents

- [screenshots](#screenshots)
- [features](#features)
- [install](#install)
  - [nixos](#nixos)
  - [aur (arch linux)](#from-the-aur-arch-linux)
  - [flatpak](#from-flatpak)
  - [manual](#manual)
- [configuration](#configuration)
- [library layout](#library-layout)
- [roadmap](readme/roadmap.md)
- [contributing](readme/contrib.md)
- [license](#license)

---

## screenshots

> screenshots and videos coming soon.

## features

| feature | description |
|---------|-------------|
| **library view** | steam-style grid of cover tiles with a searchable sidebar |
| **space theme ui** | dark, layered, rounded look inspired by [SpaceTheme](https://github.com/SpaceTheme/Steam) |
| **gamepad support** | navigate and launch with a controller (via `evdev`) |
| **cover art** | pulls art from local steam files, the steam CDN, and steamgriddb |
| **steam account** | sign in with the steam mobile app (QR code) or username/password |
| **full steam library** | shows all owned games, not just installed ones |
| **steam downloads** | download and install steam games directly |
| **store** | browse and buy games from the steam store in-app |
| **epic games** | epic games support via legendary |
| **gog** | gog support for offline installers |
| **torrents** | torrent downloads via libtorrent |
| **steam achievements** | view your achievements on the game page |
| **proton** | runs windows `.exe` games through a shared proton prefix |
| **tool updates** | keeps bundled tools up to date with live progress |

## install

### nixos

add the overlay to your flake:

```nix
inputs = {
  nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
  pawprnt pkgs = {
    url = "github:pawprnt/nixpkgs";
    inputs.nixpkgs.follows = "nixpkgs";
  };
};
```

apply the overlay in your nixos configuration:

```nix
nixpkgs.overlays = [ inputs.pawprnt-pkgs.overlays.default ];
```

then install like any other package:

```nix
environment.systemPackages = [ pkgs.forager ];
```

or run directly:

```bash
nix run github:pawprnt/nixpkgs#forager
```

### from the aur (arch linux)

```
paru -S forager
```

### from flatpak

```
flatpak remote-add --user pawprnt https://pawprnt.github.io/flatpak-repo/repo
flatpak install --user pawprnt io.github.pawprnt.forager
flatpak run io.github.pawprnt.forager
```

### manual

```
git clone https://github.com/pawprnt/forager.git
cd forager
python -m venv .venv
.venv/bin/pip install -e .
.venv/bin/forager
```

## configuration

settings are stored in `~/.config/forager/settings.json`.
cover art caches live in `~/.cache/forager/`.

environment overrides:

| variable | description | default |
|----------|-------------|---------|
| `FORAGER_CONFIG_DIR` | config directory | `~/.config/forager` |
| `FORAGER_CACHE_DIR` | cache directory | `~/.cache/forager` |
| `STEAMGRIDDB_API_KEY` | steamgriddb token fallback | — |

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

## roadmap

see [readme/roadmap.md](readme/roadmap.md) for the full roadmap.

## contributing

see [readme/contrib.md](readme/contrib.md).

## license

AGPL-3.0 — see [LICENSE](LICENSE) for details.

bundled third-party assets carry their own licenses:

| asset | license |
|-------|---------|
| [Iconoir](https://iconoir.com) | MIT — UI icons |
| [VT323](https://github.com/google/fonts/tree/main/ofl/vt323) | SIL OFL 1.1 — placeholder art font |
| [FluentSystemIcons](https://github.com/microsoft/fluentui-system-icons) | MIT — store webview icons |
| [SpaceTheme](https://github.com/SpaceTheme/Steam) | MIT — store webview styling |
