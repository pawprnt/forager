# install

## requirements

- Python 3.10+
- a steam client install (for the steam library source and local cover art)

## from the aur (arch linux)

```
paru -S forager
```

## from source

```
git clone https://github.com/pawprnt/forager.git
cd forager
python -m venv .venv
.venv/bin/pip install -e .
.venv/bin/forager
```

or run without installing:

```
QT_QPA_PLATFORM=wayland PYTHONPATH=src python -m forager
```

## system-wide (arch linux)

```
sudo pacman -S python-pyside6 python-evdev python-keyring python-pillow
git clone https://github.com/pawprnt/forager.git
cd forager
sudo python -m pip install --break-system-packages --no-deps .
forager
```

- `--break-system-packages` is required on arch (PEP 668)
- `--no-deps` keeps pacman in charge of dependencies

## release wheel

download the `.whl` from [releases](https://github.com/pawprnt/forager/releases) and:

```
pip install forager-<version>-py3-none-any.whl
forager
```

## runtime dependencies

- `PySide6` — Qt6 bindings
- `evdev` — gamepad support
- `keyring` — credential storage
- `Pillow` — image processing
- `PySide6-WebEngine` — store webview (optional)
