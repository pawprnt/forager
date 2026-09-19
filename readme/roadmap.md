# roadmap

all core features are implemented. current focus is polish, testing, and packaging for `v1.0.0`.

## done

| feature | version |
|---------|---------|
| library view | v0.1.0 |
| space theme ui | v0.1.0 |
| gamepad navigation | v0.2.0 |
| cover art pipeline | v0.2.0 |
| steam account auth | v0.3.0 |
| full steam library | v0.3.0 |
| steam downloads | v0.3.0 |
| store webview | v0.4.0 |
| epic games (legendary) | v0.4.0 |
| gog support | v0.4.0 |
| steam achievements | v0.5.0 |
| torrent downloads | v0.5.0 |
| proton management | v0.5.0 |

## in progress

| feature | target |
|---------|--------|
| itch.io store integration | v0.6.0 |
| custom themes / color schemes | v0.6.0 |
| minecraft instances & skins | v0.6.0 |

## planned

| feature | target |
|---------|--------|
| windows native support | v1.0.0 |
| plugin system | post-v1.0 |
| cloud sync (steam cloud) | post-v1.0 |

## questioning

| idea | notes |
|------|-------|
| c++ rewrite | would cut startup time, drop python + pyside6 deps, and fix gil-bound paint/jank. ~275h effort. tradeoff is loss of hot reload and harder contrib. see [discussion](https://github.com/pawprnt/forager/discussions) |
