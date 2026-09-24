# contributing to forager

thanks for thinking about contributing. this doc covers the basics.

## getting started

1. fork and clone the repo
2. check out the `cpp` branch:

```bash
cd forager
git checkout cpp
```

3. build with cmake:

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build -j$(nproc)
./build/forager
```

or use the justfile:

```bash
just build    # configure + build
just run      # build + run
just test     # run tests
```

on nixos:

```bash
nix develop   # enter dev shell
just build
just run-nix
```

## project structure

```
src/
├── app/            # application entry point, constants
├── core/           # config, game model, paths
├── library/        # scanner, launcher, playtime
├── providers/      # store backends (steam, epic, gog, torrent)
├── services/       # steamgriddb, icon provider
├── compatibility/  # proton management
├── updates/        # bundled tool auto-updater
├── ui/
│   ├── pages/      # library grid, game page, downloads, store
│   ├── widgets/    # game card, sidebar, titlebar, etc.
│   ├── dialogs/    # settings, steam auth, steamgriddb
│   └── theme.*     # space theme palette + qss
└── utils/          # network, filesystem, subprocess, secrets
```

## code style

- c++17, `#pragma once`
- camelCase functions, PascalCase classes, `m_` member prefix
- no comments unless they explain why
- match the style of the file you're editing

## making changes

1. create a branch: `git checkout -b feat/my-feature`
2. make your changes
3. run `just test` to make sure nothing broke
4. commit with a clear message
5. open a PR against `cpp`

## reporting bugs

open an issue with:

- what you expected
- what happened
- steps to reproduce
- your OS and compiler version

## license

by contributing you agree your code is licensed under AGPL-3.0.
