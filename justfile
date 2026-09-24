# forager build commands
# auto-enters nix develop on NixOS; no-op if already inside the shell

# ── helpers ────────────────────────────────────────────────────────────

@nixos:
    #!/usr/bin/env bash
    test -f /etc/NIXOS && echo "yes" || echo "no"

@_run *args:
    #!/usr/bin/env bash
    if test -n "${IN_NIX_SHELL:-}"; then
        "$@"
    elif test -f /etc/NIXOS; then
        nix develop --command "$@"
    else
        "$@"
    fi

# ── build ──────────────────────────────────────────────────────────────

default: build

configure:
    just _run cmake -B build -DCMAKE_BUILD_TYPE=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS=ON

configure-release:
    just _run cmake -B build -DCMAKE_BUILD_TYPE=Release

build: configure
    just _run cmake --build build -j$(nproc)

release: configure-release
    just _run cmake --build build -j$(nproc)

# ── run ────────────────────────────────────────────────────────────────

run: build
    just _run ./build/forager

# ── lifecycle ──────────────────────────────────────────────────────────

clean:
    rm -rf build/

rebuild: clean build

install: release
    just _run cmake --install build --prefix ~/.local

uninstall:
    rm -f ~/.local/bin/forager

# ── quality ────────────────────────────────────────────────────────────

test: build
    just _run ctest --test-dir build --output-on-failure

format:
    just _run bash -c 'find src/ \( -name "*.cpp" -o -name "*.h" \) -print0 | xargs -0 clang-format -i -style=file'

compiledb: configure
    ln -sf build/compile_commands.json .

# ── nix ────────────────────────────────────────────────────────────────

nix-build:
    nix build .#forager

dev:
    nix develop

# ── info ───────────────────────────────────────────────────────────────

info:
    @echo "cmake: $(command -v cmake || echo not found)"
    @echo "ctest: $(command -v ctest || echo not found)"
    @echo "qt6:   $(pkg-config --modversion Qt6Core 2>/dev/null || echo not found)"
    @echo "nixos: $(test -f /etc/NIXOS && echo yes || echo no)"
    @echo "shell: ${IN_NIX_SHELL:-no}"
