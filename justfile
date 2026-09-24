# forager build commands
# works on NixOS (auto-detects nix develop) and standard Linux

# ── helpers ────────────────────────────────────────────────────────────

# detect nixos
@nixos:
    #!/usr/bin/env bash
    test -f /etc/NIXOS && echo "yes" || echo "no"

# wrap a command for nixos (runs inside nix develop if on nixos)
@_run cmd:
    #!/usr/bin/env bash
    if test -f /etc/NIXOS; then
        nix develop --command bash -c '{{ cmd }}'
    else
        eval '{{ cmd }}'
    fi

# ── build ──────────────────────────────────────────────────────────────

# default: build debug
default: build

# configure cmake (debug)
configure:
    cmake -B build -DCMAKE_BUILD_TYPE=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS=ON

# configure cmake (release)
configure-release:
    cmake -B build -DCMAKE_BUILD_TYPE=Release

# build debug
build: configure
    cmake --build build -j$(nproc)

# build release
release: configure-release
    cmake --build build -j$(nproc)

# ── run ────────────────────────────────────────────────────────────────

# run the app
run: build
    ./build/forager

# run inside nix develop (for nixos — ensures qt libs are found)
run-nix:
    nix develop --command ./build/forager

# ── lifecycle ──────────────────────────────────────────────────────────

# clean build dir
clean:
    rm -rf build/

# clean + full rebuild
rebuild: clean build

# install to ~/.local
install: release
    cmake --install build --prefix ~/.local

# uninstall
uninstall:
    rm -f ~/.local/bin/forager

# ── quality ────────────────────────────────────────────────────────────

# run tests
test: build
    cd build && ctest --output-on-failure

# format source files
format:
    find src/ -name '*.cpp' -o -name '*.h' | xargs clang-format -i -style=file

# update compile_commands.json symlink
compiledb: configure
    ln -sf build/compile_commands.json .

# ── nix ────────────────────────────────────────────────────────────────

# build nix package
nix-build:
    nix build .#forager

# enter dev shell
dev:
    nix develop

# ── info ───────────────────────────────────────────────────────────────

# show build environment info
info:
    @echo "cmake: $(command -v cmake || echo not found)"
    @echo "qt6:   $(pkg-config --modversion Qt6Core 2>/dev/null || echo not found)"
    @echo "nixos: $(test -f /etc/NIXOS && echo yes || echo no)"
