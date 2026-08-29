from pathlib import Path

from forager.core.game import Game, Source


def test_search_leaf_not_used(tmp_path, monkeypatch):
    monkeypatch.setattr("forager.core.game.games_dir", lambda: tmp_path)
    game = Game(
        name="series/sequel/asylum",
        source=Source.STANDALONE,
        path=tmp_path / "standalone/series/sequel/asylum",
    )
    assert game.sgdb_search == (["sequel"], "asylum")


def test_search_skips_generic_container(tmp_path, monkeypatch):
    monkeypatch.setattr("forager.core.game.games_dir", lambda: tmp_path)
    game = Game(
        name="26.2",
        source=Source.STANDALONE,
        path=tmp_path / "minecraft/26.2",
    )
    assert game.sgdb_search is None


def test_engine_single_game_searches_own_name(tmp_path, monkeypatch):
    monkeypatch.setattr("forager.core.game.games_dir", lambda: tmp_path)
    game = Game(
        name="monster girl quest",
        source=Source.STANDALONE,
        path=tmp_path / "standalone/other/monster girl quest",
    )
    assert game.sgdb_search == (["monster girl quest"], "")


def test_direct_single_game_no_search(tmp_path, monkeypatch):
    monkeypatch.setattr("forager.core.game.games_dir", lambda: tmp_path)
    game = Game(
        name="Hades",
        source=Source.STANDALONE,
        path=tmp_path / "standalone/Hades",
    )
    assert game.sgdb_search is None


def test_search_names_wins(tmp_path, monkeypatch):
    monkeypatch.setattr("forager.core.game.games_dir", lambda: tmp_path)
    game = Game(
        name="bdcc",
        source=Source.STANDALONE,
        path=tmp_path / "standalone/bdcc",
        search_names=["Broken Dreams Correctional Center"],
    )
    assert game.sgdb_search == (["Broken Dreams Correctional Center"], "")


def test_steam_never_searches(tmp_path, monkeypatch):
    monkeypatch.setattr("forager.core.game.games_dir", lambda: tmp_path)
    game = Game(
        name="Foo",
        source=Source.STEAM,
        path=tmp_path / "steam/steamapps/common/Foo",
        app_id="123",
    )
    assert game.sgdb_search is None


def test_outside_games_dir_no_search(tmp_path, monkeypatch):
    monkeypatch.setattr("forager.core.game.games_dir", lambda: tmp_path)
    game = Game(
        name="elsewhere",
        source=Source.STANDALONE,
        path=Path("/tmp/elsewhere"),
    )
    assert game.sgdb_search is None
