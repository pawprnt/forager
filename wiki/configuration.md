# configuration

settings are stored in `~/.config/forager/settings.json`.
cover art caches live in `~/.cache/forager/`.

open the **forager -> Settings...** menu to change settings in the app.

## settings

| key | default | description |
|-----|---------|-------------|
| `games_dir` | `~/Games` | where your game library lives |
| `steam_appcache` | auto-detected | steam appcache/librarycache folder |
| `display_size` | `medium` | card size: `small` (120x180), `medium` (165x248), `large` (250x375) |
| `proton.prefix_name` | `single` | proton prefix name |
| `proton.features.rpgmaker_vxace_rtp` | `false` | add rpg maker vx ace RTP to prefix |

## environment overrides

| variable | description |
|----------|-------------|
| `FORAGER_CONFIG_DIR` | config directory (default `~/.config/forager`) |
| `FORAGER_CACHE_DIR` | cache directory (default `~/.cache/forager`) |
| `STEAMGRIDDB_API_KEY` | steamgriddb token fallback |

## credentials

steam and steamgriddb credentials are stored in your system keyring (never plaintext). use the Settings -> Account tab to sign in.
