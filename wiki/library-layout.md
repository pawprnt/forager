# library layout

your game library folder should look like this:

```
~/Games/
├── steam/
│   └── steamapps/            # appmanifest_*.acf + common/<name>
├── minecraft/                # one folder per minecraft instance
└── drm-free/
    ├── standalone/
    │   └── <engine>/         # other, rpgMaker, unity, unreal
    │       └── <game>/       # standalone games
    └── series/
        └── <engine>/
            └── <series>/
                └── <game>/   # games grouped by series
```

## game detection

games are detected by:

- an executable (`*.x86_64`, `*.sh`, `*.py`, `*.exe`)
- a `Game.ini` file

## engine folders

the `<engine>` folder groups games by engine:

- `other` — anything that doesn't fit the others
- `rpgMaker` — RPG Maker games
- `unity` — Unity games
- `unreal` — Unreal Engine games

the engine level is stripped from game names in the UI.

## series games

games in the `series/` folder are grouped by series. the game name in the UI is the joined relative path (e.g. `furry shades of gay/2`).
