import sys
from pathlib import Path

import pytest

pytest.importorskip("PySide6")

from forager.core.game import Game, Source
from forager.artwork import pipeline as art
from forager.providers.steam import appid as sid


@pytest.fixture(autouse=True)
def _isolated_cache(tmp_path, monkeypatch):
    monkeypatch.setattr(sid, "_STEAM_APPID_FILE", tmp_path / "steam_app_ids.json")
    monkeypatch.setattr(sid, "_STEAM_APPID_CACHE", {})
    yield


@pytest.fixture
def _games_dir(tmp_path, monkeypatch):
    root = tmp_path / "games"
    root.mkdir()
    monkeypatch.setattr("forager.core.game.games_dir", lambda: root)
    return root


def _game(name="Some Game", source=Source.STANDALONE, path=None):
    return Game(name, source, path or Path(f"/games/{name}"))


def test_steam_game_uses_own_app_id(_isolated_cache):
    g = _game("Portal 2", Source.STEAM)
    g.app_id = "620"
    assert art.steam_app_id(g) == "620"


def test_search_terms_prefer_search_names(_isolated_cache):
    g = _game("bdcc")
    g.search_names = ["Broken Dreams Correctional Center"]
    assert sid._steam_search_terms(g) == ["Broken Dreams Correctional Center"]


def test_search_terms_series_combined_first(_isolated_cache, _games_dir):
    g = _game("asylum", path=_games_dir / "series" / "sequel" / "asylum")
    assert sid._steam_search_terms(g) == ["sequel asylum", "asylum"]


def test_search_terms_leaf_fallback(_isolated_cache):
    assert sid._steam_search_terms(_game("Hades")) == ["Hades"]


def test_name_matches_exact(_isolated_cache):
    assert sid._name_matches("Hades", "hades")
    assert sid._name_matches("Furry Shades of Gay", "furry shades of gay")
    assert not sid._name_matches("Batman: Arkham Asylum", "asylum")
    assert not sid._name_matches("NBA 2K26", "26.2")
    assert not sid._name_matches("Counter-Strike 2", "2")


def test_name_matches_distinctive_prefix(_isolated_cache):
    assert sid._name_matches("Furry Shades of Gay 2: A Shade Gayer", "furry shades of gay 2")
    assert not sid._name_matches("Hades II", "hades")


def test_steam_lookup_cached(_isolated_cache, monkeypatch):
    g = _game("Unique Name 12345")
    calls = []
    monkeypatch.setattr(sid, "_steam_store_search", lambda term: (calls.append(term), "999")[1])
    assert art.steam_app_id(g) == "999"
    assert art.steam_app_id(g) == "999"
    assert calls == ["Unique Name 12345"]
    assert sid._STEAM_APPID_CACHE["v2:unique name 12345"] == "999"


def test_steam_lookup_negative_cached(_isolated_cache, monkeypatch):
    g = _game("No Steam Port 999999")
    calls = []
    monkeypatch.setattr(sid, "_steam_store_search", lambda term: (calls.append(term), None)[1])
    assert art.steam_app_id(g) is None
    assert art.steam_app_id(g) is None
    assert calls == ["No Steam Port 999999"]


def test_series_lookup_tries_combined_before_leaf(_isolated_cache, monkeypatch, _games_dir):
    g = _game("asylum", path=_games_dir / "series" / "sequel" / "asylum")
    calls = []
    monkeypatch.setattr(
        sid, "_steam_store_search",
        lambda term: (calls.append(term), "35140" if term == "sequel asylum" else None)[1],
    )
    assert art.steam_app_id(g) == "35140"
    assert calls == ["sequel asylum"]


def test_load_hero_prefers_steam_cdn_over_sgdb(_isolated_cache, monkeypatch):
    g = _game("Holo Hunter 42")
    order = []

    monkeypatch.setattr(art, "_cached_hero_path", lambda g: None)
    monkeypatch.setattr(art, "steam_app_id", lambda g: order.append("app_id") or "777")
    monkeypatch.setattr(
        art, "_steam_cdn_bytes",
        lambda app_id, names: order.append(("cdn", names)) or b"steam-hero",
    )
    monkeypatch.setattr(
        art, "fetch_banner_bytes_for_steam",
        lambda app_id: order.append(("sgdb-steam", app_id)) or None,
    )
    monkeypatch.setattr(
        art, "fetch_banner_bytes_for_game",
        lambda game: order.append("sgdb-search") or None,
    )
    monkeypatch.setattr(art, "load_header_bytes", lambda game, allow_network=True: None)

    data = art.load_hero_bytes(g)
    assert data == b"steam-hero"
    assert order == ["app_id", ("cdn", ("library_hero.jpg", "library_hero_blur.jpg"))]


def test_load_hero_falls_back_to_sgdb_when_no_steam_cdn(_isolated_cache, monkeypatch):
    g = _game("Holo Hunter 42")
    order = []

    monkeypatch.setattr(art, "_cached_hero_path", lambda g: None)
    monkeypatch.setattr(art, "steam_app_id", lambda g: "777")
    monkeypatch.setattr(art, "_steam_cdn_bytes", lambda app_id, names: order.append("cdn") or None)
    monkeypatch.setattr(
        art, "fetch_banner_bytes_for_steam",
        lambda app_id: order.append("sgdb-steam") or b"sgdb-banner",
    )
    monkeypatch.setattr(art, "fetch_banner_bytes_for_game", lambda game: None)
    monkeypatch.setattr(art, "load_header_bytes", lambda game, allow_network=True: None)

    data = art.load_hero_bytes(g)
    assert data == b"sgdb-banner"
    assert order == ["cdn", "sgdb-steam"]
