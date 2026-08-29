import sys

import pytest

pytest.importorskip("PySide6")

from PySide6.QtWidgets import QApplication

from forager.ui.icons import load_icon, RESOURCE_DIR
from forager.ui.theme import C

BUNDLED = {
    "settings", "arrow-left", "arrow-right", "box", "play", "stop",
    "floppy-disk", "xmark", "clock-rotate-right", "download", "folder",
    "shield", "user",
}


@pytest.fixture(scope="module")
def app():
    return QApplication.instance() or QApplication(sys.argv)


def test_all_bundled_icons_exist_on_disk():
    missing = [n for n in BUNDLED if not (RESOURCE_DIR / f"{n}.svg").is_file()]
    assert not missing


def test_icons_load_non_null(app):
    for name in BUNDLED:
        assert not load_icon(name).isNull(), name


def test_recolor_changes_stroke(app):
    white = load_icon("arrow-left", "#ffffff").pixmap(24, 24).toImage()
    black = load_icon("arrow-left", "#000000").pixmap(24, 24).toImage()
    for x in range(white.width()):
        for y in range(white.height()):
            cw = white.pixelColor(x, y)
            cb = black.pixelColor(x, y)
            if cw.alpha() > 0:
                assert cw.value() > 128
                assert cb.value() < 128
