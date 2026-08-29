import json

import pytest

pytest.importorskip("PySide6")

from forager.providers.steam import library
from forager.providers.steam.provider import SteamProvider
from forager.core.game import Game, Source


@pytest.fixture
def app_creds(monkeypatch):
    monkeypatch.setattr("forager.providers.steam.library.credentials.get_steamid", lambda: "76561197960265728")
    monkeypatch.setattr("forager.providers.steam.library.credentials.get_steam_web_api_key", lambda: "KEY")


def test_owned_games_parses(monkeypatch, app_creds):
    payload = json.dumps({
        "response": {
            "games": [
                {"appid": 440, "name": "Team Fortress 2"},
                {"appid": 10, "name": None},
            ]
        }
    }).encode()

    def fake_get(url, timeout=15):
        return payload

    monkeypatch.setattr("forager.providers.steam.library.http_get", fake_get)
    games = library.owned_games()
    assert games == [
        {"appid": "440", "name": "Team Fortress 2"},
        {"appid": "10", "name": "App 10"},
    ]


def test_owned_games_requires_key(monkeypatch):
    monkeypatch.setattr("forager.providers.steam.library.credentials.get_steamid", lambda: None)
    monkeypatch.setattr("forager.providers.steam.library.credentials.get_steam_web_api_key", lambda: None)
    assert library.owned_games() == []


def test_scanner_includes_owned(monkeypatch, app_creds):
    monkeypatch.setattr(
        "forager.providers.steam.library.owned_games",
        lambda: [{"appid": "999", "name": "Owned But Not Installed"}],
    )
    from forager.library.scanner import _scan_owned_steam

    games = _scan_owned_steam()
    assert any(g.app_id == "999" and g.installed is False and g.path is None for g in games)


def test_steam_provider_list_owned(monkeypatch, app_creds):
    monkeypatch.setattr(
        "forager.providers.steam.library.owned_games",
        lambda: [{"appid": "440", "name": "Team Fortress 2"}],
    )
    owned = SteamProvider().list_owned()
    assert owned[0].app_id == "440"
    assert owned[0].provider == "steam"
    assert owned[0].installed is False


def test_steam_provider_marks_locally_installed(monkeypatch, app_creds):
    monkeypatch.setattr(
        "forager.providers.steam.library.owned_games",
        lambda: [{"appid": "440", "name": "Team Fortress 2"}],
    )
    monkeypatch.setattr(
        "forager.library.scanner._scan_steam",
        lambda: [Game(name="Team Fortress 2", source=Source.STEAM, app_id="440", installed=True)],
    )
    owned = SteamProvider().list_owned()
    assert owned[0].installed is True


def test_game_uninstalled_display_path():
    g = Game(name="X", source=Source.STEAM, path=None, app_id="1", installed=False)
    assert g.display_path == "—"
    assert g.installed is False
