import os
import stat
import subprocess
from pathlib import Path

import pytest

from forager.core.game import Game, Source
from forager.library import launcher


def test_find_executable_script(tmp_path):
    sh = tmp_path / "game.sh"
    sh.write_text("#!/bin/sh\nexit 0\n")
    sh.chmod(sh.stat().st_mode | stat.S_IXUSR)
    assert launcher._find_executable(tmp_path) == sh


def test_find_executable_exe(tmp_path):
    exe = tmp_path / "Game.exe"
    exe.write_bytes(b"MZ")
    assert launcher._find_executable(tmp_path) == exe


def test_find_executable_none(tmp_path):
    assert launcher._find_executable(tmp_path) is None


def test_launch_steam(monkeypatch):
    calls = []
    monkeypatch.setattr(subprocess, "Popen", lambda *a, **k: calls.append((a, k)))
    game = Game(name="Foo", source=Source.STEAM, path=Path(), app_id="480")
    launcher.launch(game)
    assert calls[0][0][0] == ["steam", "steam://rungameid/480"]


def test_launch_minecraft(monkeypatch):
    calls = []
    monkeypatch.setattr(subprocess, "Popen", lambda *a, **k: calls.append((a, k)))
    game = Game(name="myworld", source=Source.MINECRAFT, path=Path())
    launcher.launch(game)
    assert calls[0][0][0] == ["prismlauncher", "-l", "myworld"]


def test_launch_standalone_sh(monkeypatch, tmp_path):
    calls = []
    monkeypatch.setattr(subprocess, "Popen", lambda *a, **k: calls.append((a, k)))
    sh = tmp_path / "game.sh"
    sh.write_text("#!/bin/sh\n")
    sh.chmod(sh.stat().st_mode | stat.S_IXUSR)
    game = Game(name="game", source=Source.STANDALONE, path=tmp_path)
    launcher.launch(game)
    assert calls[0][0][0][0] == str(sh)
    assert calls[0][1]["cwd"] == tmp_path


def test_launch_standalone_exe_uses_proton(monkeypatch, tmp_path):
    import forager.compatibility as compat_pkg
    import forager.compatibility.proton as proton_module  # noqa: F401  (creates pkg attr)

    exe = tmp_path / "Game.exe"
    exe.write_bytes(b"MZ")
    launched = []
    fake_proton = type("P", (), {"launch_exe": staticmethod(lambda d, e: launched.append((d, e)))} )()
    monkeypatch.setattr(compat_pkg, "proton", fake_proton)
    game = Game(name="game", source=Source.STANDALONE, path=tmp_path)
    launcher.launch(game)
    assert launched == [(tmp_path, exe)]


def test_launch_standalone_no_executable_is_noop(monkeypatch, tmp_path):
    monkeypatch.setattr(subprocess, "Popen", lambda *a, **k: pytest.fail("no Popen expected"))
    game = Game(name="empty", source=Source.STANDALONE, path=tmp_path)
    launcher.launch(game)


def test_launch_standalone_without_path_is_noop(monkeypatch):
    monkeypatch.setattr(subprocess, "Popen", lambda *a, **k: pytest.fail("no Popen expected"))
    game = Game(name="untracked", source=Source.STANDALONE)
    assert launcher.launch(game) is None
