<p align="center">
  <img src="docs/forager.svg" width="128" alt="forager icon" />
</p>

<h1 align="center">forager</h1>

<p align="center">
  a chill game launcher for your local library.<br/>
  steam-like vibes, space theme aesthetics, no subscription fees.
</p>

<p align="center">
  <a href="#install"><img src="https://img.shields.io/badge/install-green" alt="install" /></a>
  <a href="#features"><img src="https://img.shields.io/badge/features-blue" alt="features" /></a>
  <a href="docs/roadmap.md"><img src="https://img.shields.io/badge/roadmap-purple" alt="roadmap" /></a>
  <a href="docs/contrib.md"><img src="https://img.shields.io/badge/contributing-blueviolet" alt="contributing" /></a>
  <a href="https://github.com/pawprnt/forager/blob/main/LICENSE"><img src="https://img.shields.io/badge/license-AGPL--3.0-orange" alt="license" /></a>
</p>

<p align="center">
  <img src="https://img.shields.io/badge/platform-linux-lightgrey" alt="platform" />
  <img src="https://img.shields.io/badge/C++-17-00599C" alt="c++" />
  <img src="https://img.shields.io/badge/Qt-6-41cd52" alt="qt" />
  <img src="https://img.shields.io/badge/build-CMake-blue" alt="cmake" />
</p>

---

## table of contents

- [screenshots](#screenshots)
- [features](#features)
- [install](#install)
  - [nixos](#nixos)
  - [from source](#from-source)
  - [aur (arch linux)](#from-the-aur-arch-linux)
  - [flatpak](#from-flatpak)
- [building](#building)
- [runtime requirements](#runtime-requirements)
- [configuration](#configuration)
- [library layout](#library-layout)
- [roadmap](docs/roadmap.md)
- [contributing](docs/contrib.md)
- [license](#license)

---

## screenshots

> screenshots and videos coming soon.

## features

| feature | description |
|---------|-------------|
| **library view** | steam-style grid of cover tiles with a searchable sidebar |
| **space theme ui** | dark, layered, rounded look inspired by [SpaceTheme](https://github.com/SpaceTheme/Steam) |
| **gamepad support** | navigate and launch with a controller (via `libevdev`) |
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
| **minecraft** | launches minecraft instances directly (offline, no external launcher) |
| **tool updates** | keeps bundled tools up to date with live progress |

## install

### nixos

add the overlay to your flake:

```nix
inputs = {
  nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
  pawprnt-pkgs = {
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

### from source

requires (build): cmake, pkg-config, Qt6 (base, webengine, svg), libevdev, libsecret, qrencode, libglvnd
runtime extras (optional): see [runtime requirements](#runtime-requirements)

```bash
git clone https://github.com/pawprnt/forager.git
cd forager
git checkout cpp
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j$(nproc)
./build/forager
```

or use the justfile (auto-detects nixos and enters the dev shell when needed):

```bash
just build    # configure + build debug
just run      # build + run
just release  # build release
just test     # build + run ctest
just install  # install to ~/.local
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

## building

### requirements

- cmake >= 3.20
- a c++17 compiler (gcc or clang)
- Qt6: widgets, network, concurrent, webengine, svg
- libevdev
- libsecret
- qrencode
- libglvnd (opengl)

runtime-only extras (optional tools for specific features) are listed under [runtime requirements](#runtime-requirements).

### nixos

```bash
nix develop     # enter dev shell with all deps
just build      # build
just run        # run
```

### other linux

install dependencies via your package manager, then:

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build -j$(nproc)
./build/forager
```

## runtime requirements

these are what the installed app needs at runtime. build tools are covered under [building](#building).

### always needed (linked)

| dependency | why |
|------------|-----|
| Qt6: widgets, network, concurrent, webengine, svg | ui, networking, store webview |
| libevdev | gamepad navigation |
| libsecret | steam + steamgriddb credentials (keyring) |
| qrencode | steam qr sign-in |
| libglvnd (opengl) | rendering |

### feature-triggered (optional)

invoked only when you use the related feature. not required just to launch forager.

| dependency | needed for |
|------------|------------|
| `steam` | launching installed steam games |
| a JRE matching the minecraft version | launching minecraft instances (auto-detected; NixOS fetches into the nix store, other distros offer a package-manager install) |
| a secret service provider (e.g. keepassxc, gnome-keyring) | reading/writing credentials via libsecret |
| DepotDownloader | steam game downloads |
| steamcmd | proton updates |
| proton | running windows `.exe` games |

DepotDownloader, steamcmd, and proton are managed under forager's cache dir (`~/.cache/forager/` by default) — you don't install them system-wide. Minecraft libraries, assets, and client jar live there too (re-downloaded after cache clears on NixOS).

## configuration

settings are stored in `~/.config/forager/settings.json`.
cover art caches live in `~/.cache/forager/`.

environment overrides:

| variable | description | default |
|----------|-------------|---------|
| `FORAGER_CONFIG_DIR` | config directory | `~/.config/forager` |
| `FORAGER_CACHE_DIR` | cache directory | `~/.cache/forager` |
| `STEAMGRIDDB_API_KEY` | steamgriddb token fallback | — |

steam and steamgriddb credentials are stored in your system keyring (via libsecret).

## library layout

your game library folder should look like:

```
~/Games/
├── steam/
│   └── steamapps/
├── minecraft/
│   └── <instance>/
│       ├── mmc-pack.json
│       └── .minecraft/
└── drm-free/
    ├── standalone/
    │   └── <engine>/
    │       └── <game>/
    └── series/
        └── <engine>/
            └── <series>/
                └── <game>/
```

games are detected by an executable or `Game.ini` in the folder. minecraft instances are folders under `minecraft/`; forager launches them directly with a detected JRE (offline auth — set `FORAGER_MC_USER` to override the default `Player` name).

## roadmap

see [docs/roadmap.md](docs/roadmap.md) for the full roadmap.

## contributing

see [docs/contrib.md](docs/contrib.md).

## license

AGPL-3.0 — see [LICENSE](LICENSE) for details.

bundled third-party assets carry their own licenses:

| asset | license |
|-------|---------|
| [Iconoir](https://iconoir.com) | MIT — UI icons |
| [VT323](https://github.com/google/fonts/tree/main/ofl/vt323) | SIL OFL 1.1 — placeholder art font |
| [FluentSystemIcons](https://github.com/microsoft/fluentui-system-icons) | MIT — store webview icons |
| [SpaceTheme](https://github.com/SpaceTheme/Steam) | MIT — store webview styling |
