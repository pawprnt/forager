from __future__ import annotations
import os
import json
import urllib.parse
from pathlib import Path

from forager.core.constants import KEYRING_SERVICE
from forager.utils.network import USER_AGENT
from forager.artwork.pixmap_utils import bytes_to_pixmap

try:
    import keyring as _keyring
except Exception:
    _keyring = None

BASE = "https://www.steamgriddb.com/api/v2"
ICON_SIZE = 48

KEYRING_USER = "steamgriddb"


def get_api_key() -> str | None:
    if _keyring is not None:
        try:
            stored = _keyring.get_password(KEYRING_SERVICE, KEYRING_USER)
            if stored:
                return stored
        except Exception:
            pass
    return os.getenv("STEAMGRIDDB_API_KEY")


def set_api_key(token: str) -> None:
    if _keyring is None:
        raise RuntimeError("keyring backend unavailable")
    _keyring.set_password(KEYRING_SERVICE, KEYRING_USER, token)


def has_api_key() -> bool:
    return bool(get_api_key())


def _api_headers() -> dict[str, str]:
    headers = {
        "User-Agent": USER_AGENT,
        "Accept": "application/json",
    }
    key = get_api_key()
    if key:
        headers["Authorization"] = f"Bearer {key}"
    return headers


def _api_get(path: str) -> dict | None:
    from forager.utils.network import http_get

    try:
        raw = http_get(f"{BASE}{path}", timeout=10, headers=_api_headers())
        return json.loads(raw.decode())
    except Exception:
        return None


def _download(url: str) -> bytes | None:
    from forager.utils.network import http_get

    try:
        return http_get(url)
    except Exception:
        return None


def _asset_bytes(kind: str, sgdb_id: int, dimensions: str | None = None) -> bytes | None:
    path = f"/{kind}/game/{sgdb_id}"
    if dimensions:
        path += f"?dimensions={urllib.parse.quote(dimensions)}"
    data = _api_get(path)
    if not data or not data.get("success") or not data.get("data"):
        return None
    best = _pick_best_grid(data["data"]) if kind == "grids" else _pick_best(data["data"])
    if not best:
        return None
    raw = _download(best["url"])
    if raw:
        return raw
    thumb_url = best.get("thumb")
    if thumb_url:
        return _download(thumb_url)
    return None


def _asset_banner_bytes(sgdb_id: int) -> bytes | None:
    for dims in ("920x430", "460x215"):
        raw = _asset_bytes("grids", sgdb_id, dimensions=dims)
        if raw:
            return raw
    return None


def fetch_header_for_steam(app_id: str):
    data = fetch_header_bytes_for_steam(app_id)
    return bytes_to_pixmap(data, max_size=ICON_SIZE) if data else None


def fetch_banner_bytes_for_steam(app_id: str) -> bytes | None:
    game_data = _api_get(f"/games/steam/{app_id}")
    if not game_data or not game_data.get("success"):
        return None
    game_inner = game_data.get("data")
    sgdb_id = game_inner.get("id") if isinstance(game_inner, dict) else None
    if not sgdb_id:
        return None
    return _asset_banner_bytes(sgdb_id)


def fetch_header_bytes_for_steam(app_id: str) -> bytes | None:
    game_data = _api_get(f"/games/steam/{app_id}")
    if not game_data or not game_data.get("success"):
        return None
    game_inner = game_data.get("data")
    sgdb_id = game_inner.get("id") if isinstance(game_inner, dict) else None
    if not sgdb_id:
        return None
    return _asset_bytes("headers", sgdb_id)


def _match_entry(results: list[dict], query: str, match_term: str) -> dict | None:
    if match_term:
        for entry in results:
            if match_term.lower() in (entry.get("name") or "").lower():
                return entry
        return None
    query = query.lower()
    for entry in results:
        if query in (entry.get("name") or "").lower():
            return entry
    return None


def _search_fetch(kind: str, query: str, match_term: str, dimensions: str | None = None) -> bytes | None:
    results = _api_get(f"/search/autocomplete/{urllib.parse.quote(query)}")
    if not results or not results.get("success") or not results.get("data"):
        return None
    entry = _match_entry(results["data"], query, match_term)
    if entry is None:
        return None
    return _asset_bytes(kind, entry["id"], dimensions=dimensions)


def _asset_for_game_bytes(kind: str, game, dimensions: str | None = None) -> bytes | None:
    plan = game.sgdb_search
    if not plan:
        return None
    queries, match_term = plan
    for q in queries:
        raw = _search_fetch(kind, q, match_term, dimensions=dimensions)
        if raw:
            return raw
    return None


def fetch_grid_bytes_for_game(game) -> bytes | None:
    return _asset_for_game_bytes("grids", game)


def fetch_header_bytes_for_game(game) -> bytes | None:
    return _asset_for_game_bytes("headers", game)


def fetch_banner_bytes_for_game(game) -> bytes | None:
    plan = game.sgdb_search
    if not plan:
        return None
    queries, match_term = plan
    for q in queries:
        for dims in ("920x430", "460x215"):
            raw = _search_fetch("grids", q, match_term, dimensions=dims)
            if raw:
                return raw
    return None


def fetch_icon_bytes_for_game(game) -> bytes | None:
    return _asset_for_game_bytes("icons", game)


def fetch_icon_for_steam(app_id: str):
    data = fetch_icon_bytes_for_steam(app_id)
    return bytes_to_pixmap(data, max_size=ICON_SIZE) if data else None


def fetch_icon_bytes_for_steam(app_id: str) -> bytes | None:
    game_data = _api_get(f"/games/steam/{app_id}")
    if not game_data or not game_data.get("success"):
        return None
    game_inner = game_data.get("data")
    sgdb_id = game_inner.get("id") if isinstance(game_inner, dict) else None
    if not sgdb_id:
        return None
    return _asset_bytes("icons", sgdb_id)


def fetch_grid_bytes_for_steam(app_id: str) -> bytes | None:
    data = _api_get(f"/grids/steam/{app_id}")
    if not data or not data.get("success") or not data.get("data"):
        return None
    best = _pick_best_grid(data["data"])
    if not best:
        return None
    raw = _download(best["url"])
    if raw:
        return raw
    thumb_url = best.get("thumb")
    if thumb_url:
        return _download(thumb_url)
    return None


def _pick_best(icons: list[dict]) -> dict | None:
    def key(icon: dict) -> tuple:
        return (
            0 if icon.get("style") == "official" else 1,
            -icon.get("upvotes", 0),
        )
    return min(icons, key=key) if icons else None


def _pick_best_grid(grids: list[dict]) -> dict | None:
    def key(grid: dict) -> tuple:
        dims_ok = grid.get("width") == 600 and grid.get("height") == 900
        return (
            0 if grid.get("style") == "official" else 1,
            0 if dims_ok else 1,
            -grid.get("upvotes", 0),
        )
    return min(grids, key=key) if grids else None
