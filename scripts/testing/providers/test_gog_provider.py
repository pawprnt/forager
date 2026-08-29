from __future__ import annotations

import json
from pathlib import Path

import pytest

from forager.providers import gog  # noqa: F401  (imports provider, registers)
from forager.providers.base import PROVIDERS
from forager.providers.gog import provider as gog_provider
from forager.providers.gog.provider import GogProvider
from forager.providers.steam import credentials as creds


_SAMPLE_PRODUCTS = {
    "products": [
        {"id": 1207658922, "title": "The Witcher 3"},
        {"id": 1135233430, "title": "Cyberpunk 2077"},
    ]
}


@pytest.fixture
def no_token(monkeypatch):
    monkeypatch.setattr(gog_provider, "get_gog_token", lambda: None)


@pytest.fixture
def with_token(monkeypatch):
    monkeypatch.setattr(gog_provider, "get_gog_token", lambda: "fake-token")


def test_registered():
    assert "gog" in PROVIDERS


def test_is_configured(no_token):
    assert GogProvider().is_configured() is False


def test_is_configured_true(with_token):
    assert GogProvider().is_configured() is True


def test_list_owned_empty_when_no_token(no_token):
    assert GogProvider().list_owned() == []


def test_list_owned_parses(with_token, monkeypatch):
    body = json.dumps(_SAMPLE_PRODUCTS).encode("utf-8")
    monkeypatch.setattr(gog_provider, "http_get", lambda url: body)
    games = GogProvider().list_owned()
    assert len(games) == 2
    assert games[0].app_id == "1207658922"
    assert games[0].name == "The Witcher 3"
    assert games[0].provider == "gog"
    assert games[0].installed is False
