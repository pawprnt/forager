"""Steam game downloads (roadmap item 3).

Drives DepotDownloader (already vendored for Steam login) with the stored
credentials to download an owned app to a destination folder, streaming
progress back through a callback.  Network/disk work happens in a worker
thread (see ``forager.ui.workers.DownloadWorker``); this module only builds
and runs the DepotDownloader command.
"""
from __future__ import annotations

from pathlib import Path

from forager.compatibility.proton import (
    DownloadProgress,
    DownloadCancelled,
    _PROGRESS_RE,
)
from forager.providers.base import BackendNotConfigured
from forager.providers.steam import credentials, depotdownloader


def _download_cmd(app_id: str, destination: Path, username: str, password: str | None) -> list[str]:
    cmd = [
        str(depotdownloader.depotdownloader_bin()),
        "-app", str(app_id),
        "-dir", str(destination),
        "-username", username,
        "-remember-password",
    ]
    if password:
        cmd += ["-password", password]
    return cmd


def download_app(app_id, destination, on_progress=None, cancel=None) -> None:
    if not credentials.has_credentials():
        raise BackendNotConfigured("Sign in to Steam to download games.")
    username = credentials.get_username()
    if not username:
        raise BackendNotConfigured("Sign in to Steam to download games.")

    dest = Path(destination)
    dest.mkdir(parents=True, exist_ok=True)

    cmd = _download_cmd(app_id, dest, username, credentials.get_password())

    def on_line(line: str) -> None:
        if on_progress is None:
            return
        m = _PROGRESS_RE.search(line)
        if m:
            stage, percent, done, total = (
                m.group(1),
                float(m.group(2)),
                int(m.group(3)),
                int(m.group(4)),
            )
            on_progress(DownloadProgress(stage, percent, done, total, 0))

    log, tail, code, cancelled = depotdownloader._run_dd(
        cmd, timeout=3600.0, cancel_event=cancel, on_line=on_line
    )
    if cancelled:
        raise DownloadCancelled()
    if code != 0:
        raise RuntimeError("DepotDownloader failed: " + (tail or "see log").strip())
