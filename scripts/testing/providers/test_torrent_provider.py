"""Tests for the libtorrent torrent backend."""

import pytest

lt = pytest.importorskip("libtorrent")

from forager.providers import torrent  # noqa: E402  (imports + registers TorrentProvider)
from forager.providers.base import PROVIDERS  # noqa: E402


def test_registered():
    assert "torrent" in PROVIDERS


def test_is_configured_when_libtorrent_present():
    provider = torrent.TorrentProvider()
    assert provider.is_configured() is True


def test_list_owned_empty():
    provider = torrent.TorrentProvider()
    assert provider.list_owned() == []
