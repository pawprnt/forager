# install

## requirements

- cmake >= 3.20
- a c++17 compiler (gcc or clang)
- qt6: widgets, network, concurrent, webengine, svg
- libevdev
- libsecret
- qrencode
- libglvnd (opengl)

## from the aur (arch linux)

```
paru -S forager
```

## from source

```
git clone https://github.com/pawprnt/forager.git
cd forager
git checkout cpp
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j$(nproc)
./build/forager
```

or use the justfile:

```
just build    # configure + build debug
just run      # build + run
just release  # build release
just install  # install to ~/.local
```

## nixos

```bash
nix develop     # enter dev shell with all deps
just build      # build
just run-nix    # run with correct qt libs
```

or install system-wide via the overlay (see README).

## runtime dependencies

- **qt6** — widgets, network, concurrent, webengine, svg
- **libevdev** — gamepad support
- **libsecret** — credential storage (keyring)
- **qrencode** — qr code generation
- **libglvnd** — opengl
