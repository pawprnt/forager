import urllib.request

import pytest

pytest.importorskip("PySide6")

from forager.providers.epic import provider as epic_provider


def test_list_owned_parses_title_with_parentheses(monkeypatch):
    monkeypatch.setattr("shutil.which", lambda name: "/usr/bin/legendary")
    text = (
        "[INFO] Available games:\n"
        "* Half-Life (Beta) (hlbeta123)\n"
        "* Game (123456) [Windows]\n"
    )

    def fake_run(cmd, *a, **k):
        class _CP:
            stdout = text
            returncode = 0
        return _CP()

    monkeypatch.setattr("subprocess.run", fake_run)
    games = epic_provider.EpicProvider().list_owned()
    by_id = {g.app_id: g for g in games}
    assert by_id["hlbeta123"].name == "Half-Life (Beta)"
    assert by_id["123456"].name == "Game"
