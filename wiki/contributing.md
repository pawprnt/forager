# contributing

## code style

- c++17, no extensions
- `#pragma once` for headers
- camelCase for functions, PascalCase for classes
- `m_` prefix for member variables
- no comments unless they explain why
- follow the existing package layout

## where to put things

- `src/app/` — application entry point, constants
- `src/core/` — config, game model, paths
- `src/library/` — scanner, launcher, playtime
- `src/providers/` — steam, epic, gog, torrent
- `src/services/` — steamgriddb, icon provider
- `src/compatibility/` — proton
- `src/updates/` — tool updates
- `src/ui/` — theme, pages, widgets, dialogs
- `src/utils/` — network, filesystem, subprocess, secrets

## building

```bash
just build      # configure + build debug
just test       # run tests
just format     # format source files
```

## testing

```bash
just test
```

## commits

keep it chill:

```
fix: sidebar was being weird
feat: library talks to steam now
chore: tests reorganized
```

lowercase, no periods, casual tone.
