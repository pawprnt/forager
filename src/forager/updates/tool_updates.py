"""Update detection for the third-party tools forager provisions.

Only DepotDownloader is checked: it is pinned to a fixed GitHub release
(``DEPOTDL_TAG``) and GitHub publishes a versioned tag per release, so a newer
one is always discoverable. steamcmd is deliberately not checked — Valve ships
a single unversioned tarball and the binary self-updates on every run, so the
copy on disk is always current after any use.
"""
from __future__ import annotations
import json
import shutil
from dataclasses import dataclass
from pathlib import Path

from forager.compatibility.proton import (
    DEPOTDL_DIR,
    depotdl_url,
    depotdownloader_bin,
    runtime_dir,
    _flatten_depotdownloader,
)

GITHUB_LATEST_URL = (
    "https://api.github.com/repos/SteamRE/DepotDownloader/releases/latest"
)


@dataclass
class ToolUpdate:
    name: str
    installed: str | None
    latest: str


def installed_depotdl_tag() -> str | None:
    version_file = DEPOTDL_DIR / "version.txt"
    if not version_file.is_file():
        return None
    try:
        text = version_file.read_text("utf-8", errors="replace").strip()
    except OSError:
        return None
    return text or None


def _latest_depotdl_tag() -> str | None:
    from forager.utils.network import http_get

    try:
        data = json.loads(http_get(GITHUB_LATEST_URL, timeout=10))
    except Exception:
        return None
    tag = data.get("tag_name")
    return tag if isinstance(tag, str) and tag else None


def check_tool_updates() -> list[ToolUpdate]:
    """Tools with a newer release available (network failure => no updates)."""
    latest = _latest_depotdl_tag()
    if latest is None:
        return []
    installed = installed_depotdl_tag()
    if installed == latest:
        return []
    return [ToolUpdate("DepotDownloader", installed, latest)]


def update_tool_updates(report=None) -> list[str]:
    """Fetch the latest release of every outdated tool. Returns updated names."""
    updated: list[str] = []
    for update in check_tool_updates():
        if report is not None:
            report(f"Updating {update.name}...")
        _download_depotdl(update.latest)
        updated.append(update.name)
    return updated


def _download_depotdl(tag: str) -> None:
    from forager.utils.download import download_and_extract_zip

    DEPOTDL_DIR.mkdir(parents=True, exist_ok=True)
    saved_session = None
    account_cfg = DEPOTDL_DIR / "account.config"
    if account_cfg.is_file():
        try:
            saved_session = account_cfg.read_bytes()
        except OSError:
            pass
    for old in DEPOTDL_DIR.iterdir():
        if old.is_dir():
            shutil.rmtree(old, ignore_errors=True)
        else:
            old.unlink()
    download_and_extract_zip(depotdl_url(tag), DEPOTDL_DIR)
    _flatten_depotdownloader()
    depotdownloader_bin().chmod(0o755)
    (DEPOTDL_DIR / "version.txt").write_text(tag, "utf-8")
    if saved_session is not None:
        try:
            account_cfg.write_bytes(saved_session)
        except OSError:
            pass
