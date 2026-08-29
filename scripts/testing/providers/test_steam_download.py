import pytest

pytest.importorskip("PySide6")

from forager.providers.steam import downloader
from forager.compatibility.proton import DownloadProgress, DownloadCancelled


def test_download_requires_credentials(monkeypatch):
    monkeypatch.setattr("forager.providers.steam.downloader.credentials.has_credentials", lambda: False)
    with pytest.raises(Exception):
        downloader.download_app("440", "/tmp/does-not-matter")


def test_download_streams_progress(monkeypatch, tmp_path):
    monkeypatch.setattr("forager.providers.steam.downloader.credentials.has_credentials", lambda: True)
    monkeypatch.setattr("forager.providers.steam.downloader.credentials.get_username", lambda: "tester")
    monkeypatch.setattr("forager.providers.steam.downloader.credentials.get_password", lambda: None)
    monkeypatch.setattr(
        "forager.providers.steam.depotdownloader.depotdownloader_bin",
        lambda: "/usr/bin/dotnet",
    )

    progress: list[DownloadProgress] = []

    def fake_run_dd(cmd, timeout=3600.0, cancel_event=None, on_line=None):
        if on_line is not None:
            on_line("Update state (0x1) downloading, progress: 50.0 (500 / 1000)")
            on_line("Update state (0x2) complete, progress: 100.0 (1000 / 1000)")
        return (["ok"], "", 0, False)

    monkeypatch.setattr("forager.providers.steam.depotdownloader._run_dd", fake_run_dd)

    downloader.download_app("440", str(tmp_path), on_progress=progress.append)
    assert len(progress) == 2
    assert progress[0].percent == 50.0
    assert progress[-1].percent == 100.0


def test_download_cancelled(monkeypatch, tmp_path):
    monkeypatch.setattr("forager.providers.steam.downloader.credentials.has_credentials", lambda: True)
    monkeypatch.setattr("forager.providers.steam.downloader.credentials.get_username", lambda: "tester")
    monkeypatch.setattr("forager.providers.steam.downloader.credentials.get_password", lambda: None)
    monkeypatch.setattr(
        "forager.providers.steam.depotdownloader.depotdownloader_bin",
        lambda: "/usr/bin/dotnet",
    )

    def fake_run_dd(cmd, timeout=3600.0, cancel_event=None, on_line=None):
        return (["x"], "interrupted", 1, True)

    monkeypatch.setattr("forager.providers.steam.depotdownloader._run_dd", fake_run_dd)
    with pytest.raises(DownloadCancelled):
        downloader.download_app("440", str(tmp_path))
