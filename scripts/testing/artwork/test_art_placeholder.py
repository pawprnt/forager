import sys
from pathlib import Path

import pytest

pytest.importorskip("PySide6")

from PySide6.QtWidgets import QApplication

from forager.core.game import Game, Source
from forager.artwork import pipeline as art
from forager.artwork import placeholder


@pytest.fixture(scope="module")
def app():
    return QApplication.instance() or QApplication(sys.argv)


def _game(name="No Art Game", source=Source.STANDALONE, path=None):
    return Game(name, source, path or Path(f"/games/{name}"))


def test_bundled_font_file_present():
    assert placeholder._FONT_FILE.is_file()


def test_register_placeholder_font(app):
    assert art.register_placeholder_font() == "VT323"


def test_placeholder_card_nonnull(app):
    pix = art.placeholder_card(_game("No Art 1"), 900, 420)
    assert not pix.isNull()
    assert (pix.width(), pix.height()) == (900, 420)


def test_placeholder_grid_nonnull(app):
    pix = art.placeholder_grid(_game("No Art 2"), 165, 248)
    assert not pix.isNull()
    assert (pix.width(), pix.height()) == (165, 248)


def test_placeholder_card_draws_name(app):
    pix = art.placeholder_card(_game("Rando Game 3"), 900, 420)
    img = pix.toImage()
    assert img.width() > 0 and img.height() > 0
