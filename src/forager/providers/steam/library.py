"""Full Steam library retrieval (roadmap item 2).

Uses the Steam Web API ``ISteamApps/GetOwnedGames`` with the configured Web API
key + SteamID (see ``forager.providers.steam.credentials``).  Merged into the
local library so owned-but-uninstalled titles are listed too.
"""
from __future__ import annotations

import json

from forager.providers.steam import credentials
from forager.utils.network import http_get

_STEAM_API = "https://api.steampowered.com/ISteamApps/GetOwnedGames/v1/"


def owned_games(steamid: str | None = None, api_key: str | None = None) -> list[dict]:
    steamid = steamid or credentials.get_steamid()
    api_key = api_key or credentials.get_steam_web_api_key()
    if not steamid or not api_key:
        return []

    url = (
        f"{_STEAM_API}?key={api_key}&steamid={steamid}"
        "&include_appinfo=true&include_played_free_games=true&format=json"
    )
    try:
        raw = http_get(url)
    except Exception:
        return []

    try:
        data = json.loads(raw)
    except (ValueError, TypeError):
        return []

    games = data.get("response", {}).get("games", [])
    out: list[dict] = []
    for entry in games:
        appid = entry.get("appid")
        if appid is None:
            continue
        out.append({"appid": str(appid), "name": entry.get("name") or f"App {appid}"})
    return out
