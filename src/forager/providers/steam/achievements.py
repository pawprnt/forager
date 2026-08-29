"""Steam achievements (roadmap item 7).

Implements offline, keyless retrieval of a player's achievements from the local
``userdata/<steamid>/<appid>/achievements.vdf`` file written by the Steam client.
See ``docs/providers/steam.md``.
"""
from __future__ import annotations

import re
from pathlib import Path

from forager.core.paths import games_dir

_TOKEN_RE = re.compile(r'"[^"]*"|[{}]')


def _tokenize(text: str) -> list[str]:
    return [t if t in "{}" else t[1:-1] for t in _TOKEN_RE.findall(text)]


def parse_vdf(text: str) -> dict:
    """Parse a minimal KeyValues/VDF document into nested dicts.

    Handles quoted keys/values and ``{ }`` grouping, which is all the
    ``achievements.vdf`` format needs.
    """
    tokens = _tokenize(text)
    pos = 0

    def parse_object() -> dict:
        nonlocal pos
        obj: dict = {}
        while pos < len(tokens):
            tok = tokens[pos]
            if tok == "}":
                pos += 1
                return obj
            key = tok
            pos += 1
            if pos < len(tokens) and tokens[pos] == "{":
                pos += 1
                obj[key] = parse_object()
            else:
                value = tokens[pos] if pos < len(tokens) else ""
                pos += 1
                obj[key] = value
        return obj

    return parse_object()


def player_achievements(
    steamid: str, app_id: str, steam_root: str | Path | None = None
) -> list[dict]:
    """Return earned achievements for a player/app from the local VDF file.

    Returns an empty list when the file is missing or unreadable. Each entry is
    ``{"name": str, "icon": str, "achieved": bool}`` in display order.
    """
    root = Path(steam_root) if steam_root else (games_dir() / "steam")
    vdf = root / "userdata" / str(steamid) / str(app_id) / "achievements.vdf"
    if not vdf.is_file():
        return []
    try:
        data = parse_vdf(vdf.read_text(encoding="utf-8", errors="replace"))
    except Exception:
        return []
    inner = data.get("achievements", {})
    out: list[dict] = []
    for entry in inner.values():
        if not isinstance(entry, dict):
            continue
        out.append(
            {
                "name": entry.get("name", ""),
                "icon": entry.get("path", ""),
                "achieved": str(entry.get("achieved", "0")) == "1",
            }
        )
    return out


def achievement_summary(achievements: list[dict]) -> tuple[int, int, float]:
    """Return ``(earned, total, fraction)`` for a list of achievements."""
    total = len(achievements)
    if total == 0:
        return (0, 0, 0.0)
    earned = sum(1 for a in achievements if a.get("achieved"))
    return (earned, total, earned / total)
