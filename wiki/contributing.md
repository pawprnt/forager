# contributing

## code style

- `from __future__ import annotations` at the top of every module
- type hints everywhere
- no comments unless they explain why
- follow the existing package layout

## where to put things

- `core/` — config, constants, game dataclass
- `library/` — scanner, launcher, playtime
- `providers/` — steam, epic, gog, torrent
- `services/` — steamgriddb, icon provider
- `compatibility/` — proton
- `updates/` — tool updates
- `ui/` — themes, pages, widgets, dialogs

## testing

```
QT_QPA_PLATFORM=offscreen PYTHONPATH=src python -m pytest -q
```

## commits

keep it chill:

```
fix: sidebar was being weird
feat: library talks to steam now
chore: tests reorganized
```

lowercase, no periods, casual tone.
