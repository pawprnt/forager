# features

## library

- steam-style grid of cover tiles
- searchable sidebar game list
- recently played row at the top
- multiple card sizes (small, medium, large)
- gamepad navigation (via `evdev`)

## cover art

pulls art from multiple sources, in order:

1. local steam appcache
2. art cache on disk
3. steam CDN (for resolved app IDs)
4. steamgriddb (by steam app ID)
5. steamgriddb (by game search)
6. generated placeholder (sunburst/glow)

## steam integration

- sign in with QR code or username/password
- full library of all owned games (not just installed)
- download and install steam games
- view steam achievements
- browse and buy games from the store

## other providers

- **epic games** — via legendary (open-source EGS CLI)
- **gog** — offline installers via gog web API
- **torrents** — via libtorrent

## proton

- runs windows `.exe` games through a shared proton prefix
- rpg maker vx ace RTP support
- automatic proton updates via steamcmd

## ui

- space theme aesthetics (dark, layered, rounded)
- settings dialog with gamepad support
- downloads page with live progress
- game page with banner art, achievements, and info
