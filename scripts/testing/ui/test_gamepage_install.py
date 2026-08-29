import sys

import pytest

pytest.importorskip("PySide6")

from PySide6.QtWidgets import QApplication
from pathlib import Path

from forager.core.game import Game, Source
from forager.ui.pages.gamepage import GamePage


@pytest.fixture(scope="module")
def app():
    return QApplication.instance() or QApplication(sys.argv)


def test_uninstalled_game_shows_install(app):
    page = GamePage()
    game = Game(name="Not Installed", source=Source.STEAM, path=None, app_id="999", installed=False)
    page.set_game(game)
    page.set_running(False)
    assert page._play_text.text() == "Install"
    assert page._on_play.__name__ == "_on_play"
    page.deleteLater()


def test_installed_game_shows_play(app):
    page = GamePage()
    game = Game(name="Installed", source=Source.STEAM, path=Path("/tmp/forager-test-installed"), app_id="10", installed=True)
    page.set_game(game)
    page.set_running(False)
    assert page._play_text.text() == "Play"
    page.deleteLater()
