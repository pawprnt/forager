import sys
from pathlib import Path

import pytest

pytest.importorskip("PySide6")

from PySide6.QtWidgets import QApplication
from PySide6.QtGui import QFontDatabase

from forager.ui import fonts
from forager.ui.fonts import UI_FONT


@pytest.fixture(scope="module")
def app():
    return QApplication.instance() or QApplication(sys.argv)


def _repo_root() -> Path:
    p = Path(__file__).resolve()
    while p != p.parent:
        if (p / "pyproject.toml").is_file():
            return p
        p = p.parent
    return Path(__file__).resolve().parents[2]


def test_bundled_font_files_present():
    fonts_dir = _repo_root() / "src" / "forager" / "assets" / "fonts"
    for name in fonts._WEIGHT_FILES:
        assert (fonts_dir / name).is_file(), name
    assert (fonts_dir / "OFL-BeVietnamPro.txt").is_file()


def test_register_ui_font(app):
    assert fonts.register_ui_font() == "Be Vietnam Pro"


def test_ui_font_family_registered(app):
    fonts.register_ui_font()
    families = set()
    for name in fonts._WEIGHT_FILES:
        fid = QFontDatabase.addApplicationFont(
            str(_repo_root() / "src" / "forager" / "assets" / "fonts" / name)
        )
        if fid != -1:
            families.update(QFontDatabase.applicationFontFamilies(fid))
    assert UI_FONT in families
