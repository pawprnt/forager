import subprocess
import shutil

from forager.providers import epic as epic_pkg
from forager.providers.base import PROVIDERS
from forager.providers.epic import provider as epic_provider
from forager.providers.base import BackendNotConfigured, DownloadProgress


OWNED_TEXT = (
    "Legendary v0.20.34 - \"some codename\"\n"
    "[INFO] Available games:\n"
    "* Cave Story+ (abc123)\n"
    "* Hyper Light Drifter (def456)\n"
    "[INFO] Done.\n"
)

INSTALLED_TEXT = (
    "[INFO] Installed games:\n"
    "* Cave Story+ (abc123)\n"
)


def _completed(stdout, returncode=0):
    class _CP:
        pass

    cp = _CP()
    cp.stdout = stdout
    cp.returncode = returncode
    return cp


def test_list_owned_parses(monkeypatch):
    monkeypatch.setattr(shutil, "which", lambda name: "/usr/bin/legendary")

    def fake_run(cmd, *a, **k):
        if "list-installed" in cmd:
            return _completed(INSTALLED_TEXT)
        return _completed(OWNED_TEXT)

    monkeypatch.setattr(subprocess, "run", fake_run)

    games = epic_provider.EpicProvider().list_owned()
    assert len(games) == 2
    by_id = {g.app_id: g for g in games}
    assert by_id["abc123"].name == "Cave Story+"
    assert by_id["abc123"].provider == "epic"
    assert by_id["abc123"].installed is True
    assert by_id["def456"].installed is False


def test_is_configured_false_when_missing(monkeypatch):
    monkeypatch.setattr(shutil, "which", lambda name: None)
    assert epic_provider.EpicProvider().is_configured() is False


def test_provider_registered(monkeypatch):
    # Importing the subpackage registers EpicProvider.
    monkeypatch.setattr(shutil, "which", lambda name: "/usr/bin/legendary")
    from forager.providers import epic  # noqa: F401

    assert "epic" in PROVIDERS
    assert PROVIDERS["epic"] is epic_provider.EpicProvider


def test_download_raises_when_not_configured(monkeypatch):
    monkeypatch.setattr(shutil, "which", lambda name: None)
    try:
        epic_provider.EpicProvider().download("abc123", "/tmp/x")
        assert False, "expected BackendNotConfigured"
    except BackendNotConfigured:
        pass


def test_download_emits_progress(monkeypatch):
    monkeypatch.setattr(shutil, "which", lambda name: "/usr/bin/legendary")

    class _LineIO:
        def __init__(self):
            self.lines = [
                "[Worker] Downloading foo: 50.0% (1.0/2.0 MB)\n",
                "[Worker] Downloading foo: 100.0% (2.0/2.0 MB)\n",
            ]

        def __iter__(self):
            return iter(self.lines)

    class _FakeProc:
        stdout = _LineIO()

        def poll(self):
            return 0

        def wait(self, timeout=None):
            return 0

        def kill(self):
            pass

    def fake_popen(cmd, *a, **k):
        assert cmd[1] == "install"
        assert cmd[2] == "abc123"
        assert "--install-dir" in cmd
        return _FakeProc()

    monkeypatch.setattr(subprocess, "Popen", fake_popen)

    progress = []
    epic_provider.EpicProvider().download(
        "abc123", "/tmp/x", on_progress=progress.append
    )
    assert len(progress) == 2
    assert all(isinstance(p, DownloadProgress) for p in progress)
    assert progress[0].percent == 50.0
    assert progress[1].percent == 100.0
