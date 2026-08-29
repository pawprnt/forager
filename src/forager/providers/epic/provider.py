"""Epic Games backend driven by the Legendary CLI (roadmap: Epic Games).

Everything talks to Legendary via ``subprocess`` — nothing here imports or
links against Legendary, so the module stays importable even when the
``legendary`` binary is not installed. See ``docs/providers/epic.md``.
"""
from __future__ import annotations

import re
import shutil
import subprocess

from forager.providers.base import (
    Provider,
    BackendNotConfigured,
    DownloadProgress,
    OwnedGame,
    register_provider,
)


def _legendary_bin() -> str | None:
    return shutil.which("legendary")


_GAME_RE = re.compile(r"^\*?\s*(.+)\s*\(([^()]+)\)[^\n]*$")
_OWNED_MARKER = re.compile(r"available games", re.IGNORECASE)
_INSTALLED_MARKER = re.compile(r"installed games", re.IGNORECASE)
_PERCENT_RE = re.compile(r"(\d{1,3}(?:\.\d+)?)\s*%")


def _parse_games(text: str, installed_default: bool) -> list[OwnedGame]:
    """Parse Legendary ``list-games`` / ``list-installed`` text.

    Each game line looks like ``* Name (appid)`` optionally followed by
    metadata. Defensive: lines that don't yield a name + appid are skipped.
    """
    games: list[OwnedGame] = []
    for line in text.splitlines():
        stripped = line.strip()
        if not stripped or stripped.startswith("[") or stripped.startswith("Legendary"):
            continue
        if _OWNED_MARKER.search(stripped) or _INSTALLED_MARKER.search(stripped):
            continue
        m = _GAME_RE.search(stripped)
        if not m:
            continue
        name = m.group(1).strip().lstrip("*").strip().rstrip("*").strip()
        app_id = m.group(2).strip()
        if not name or not app_id:
            continue
        games.append(
            OwnedGame(
                app_id=app_id,
                name=name,
                provider="epic",
                installed=installed_default,
            )
        )
    return games


def _run_legendary(args: list[str]) -> str:
    """Run ``legendary <args>`` and return combined stdout. Empty on failure."""
    binpath = _legendary_bin()
    if binpath is None:
        return ""
    try:
        proc = subprocess.run(
            [binpath, *args],
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            text=True,
            errors="replace",
            check=False,
        )
    except (OSError, subprocess.SubprocessError):
        return ""
    if proc.returncode != 0:
        return ""
    return proc.stdout or ""


def _merge_installed(owned: list[OwnedGame], installed: list[OwnedGame]) -> list[OwnedGame]:
    installed_ids = {g.app_id for g in installed}
    for g in owned:
        if g.app_id in installed_ids:
            g.installed = True
    return owned


@register_provider
class EpicProvider(Provider):
    name = "epic"

    def is_configured(self) -> bool:
        return _legendary_bin() is not None

    def list_owned(self, account=None) -> list[OwnedGame]:
        if not self.is_configured():
            return []
        owned_text = _run_legendary(["list-owners"])
        installed_text = _run_legendary(["list-installed"])

        owned = _parse_games(owned_text, installed_default=False) if owned_text else []
        installed = (
            _parse_games(installed_text, installed_default=True)
            if installed_text
            else []
        )

        if owned:
            return _merge_installed(owned, installed)
        if installed:
            return installed
        return []

    def download(
        self,
        app_id: str,
        destination: str,
        on_progress=None,
        cancel=None,
    ) -> None:
        binpath = _legendary_bin()
        if binpath is None:
            raise BackendNotConfigured("legendary is not installed / not on PATH")

        cmd = [
            binpath,
            "install",
            app_id,
            "--yes",
            "-y",
            "--install-dir",
            str(destination),
        ]
        proc = subprocess.Popen(
            cmd,
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            text=True,
            errors="replace",
            bufsize=1,
        )
        assert proc.stdout is not None
        try:
            for line in proc.stdout:
                if cancel is not None and getattr(cancel, "is_set", lambda: False)():
                    proc.terminate()
                    try:
                        proc.wait(timeout=5)
                    except subprocess.TimeoutExpired:
                        proc.kill()
                        proc.wait()
                    return
                m = _PERCENT_RE.search(line)
                if m and on_progress is not None:
                    try:
                        percent = float(m.group(1))
                    except ValueError:
                        percent = 0.0
                    on_progress(DownloadProgress("install", percent, 0, 0, 0))
            returncode = proc.wait()
            if returncode != 0:
                raise RuntimeError(f"legendary install exited with code {returncode}")
        finally:
            if proc.poll() is None:
                proc.kill()
                proc.wait()
