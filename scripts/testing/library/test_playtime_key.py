import pytest

pytest.importorskip("PySide6")

from forager.core.game import Game, Source
from forager.library.playtime import game_key


def test_game_key_without_path_or_appid_does_not_crash():
    g = Game(name="Mystery", source=Source.STANDALONE, path=None, installed=False)
    key = game_key(g)
    assert "Mystery" in key


def test_game_key_uses_appid_when_present():
    g = Game(name="X", source=Source.STEAM, path=None, app_id="440", installed=False)
    assert game_key(g) == "steam:440"
