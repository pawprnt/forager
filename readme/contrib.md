# contributing to forager

thanks for thinking about contributing. this doc covers the basics.

## getting started

1. fork and clone the repo
2. set up a dev environment:

```bash
cd forager
python -m venv .venv
.venv/bin/pip install -e ".[dev]"
```

3. run the app:

```bash
.venv/bin/forager
```

4. run the tests:

```bash
.venv/bin/pytest
```

## project structure

```
src/forager/
├── core/           # config, constants, game model, paths
├── library/        # scanner, launcher, playtime tracking
├── artwork/        # cover art pipeline, caching, placeholders
├── providers/      # store backends (steam, epic, gog, torrent)
├── services/       # steamgriddb, icon provider
├── compatibility/  # proton management
├── updates/        # bundled tool auto-updater
├── ui/
│   ├── pages/      # library grid, game page, downloads, store
│   ├── widgets/    # game card, sidebar, titlebar, etc.
│   ├── dialogs/    # settings, steam auth, steamgriddb
│   └── theme.py    # space theme palette + qss
└── utils/          # network, filesystem, threading helpers
```

## code style

- follow existing conventions (snake_case, no comments unless needed)
- match the style of the file you're editing
- keep responses concise in PRs too

## making changes

1. create a branch: `git checkout -b feat/my-feature`
2. make your changes
3. run `pytest` to make sure nothing broke
4. commit with a clear message
5. open a PR against `main`

## reporting bugs

open an issue with:

- what you expected
- what happened
- steps to reproduce
- your OS and python version

## license

by contributing you agree your code is licensed under AGPL-3.0.
