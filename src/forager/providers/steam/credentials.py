"""Steam credential management via the system keyring.

Handles storing, retrieving, and clearing Steam usernames, passwords,
login methods, SteamIDs, and login-secure cookies.  No network or
subprocess work lives here — that belongs in ``depotdownloader``.
"""
from __future__ import annotations

import html
import re

from forager.core.constants import KEYRING_SERVICE

try:
    import keyring as _keyring
except ImportError:
    _keyring = None

KEYRING_USERNAME_KEY = "steam_username"
KEYRING_PASSWORD_KEY = "steam_password"
KEYRING_LOGIN_METHOD_KEY = "steam_login_method"
KEYRING_STEAMID_KEY = "steamid"
KEYRING_LOGIN_SECURE_KEY = "steam_login_secure"
KEYRING_STEAM_API_KEY = "steam_web_api_key"

KEYRING_GOG_TOKEN_KEY = "gog_token"


# ── helpers ────────────────────────────────────────────────────────────

def _keyring_get(key: str) -> str | None:
    if _keyring is None:
        return None
    try:
        stored = _keyring.get_password(KEYRING_SERVICE, key)
        if stored:
            return stored
    except Exception:
        pass
    return None


def _keyring_set(key: str, value: str) -> None:
    if _keyring is None:
        raise RuntimeError("keyring backend unavailable")
    _keyring.set_password(KEYRING_SERVICE, key, value)


def _keyring_delete(key: str) -> None:
    if _keyring is None:
        return
    try:
        _keyring.delete_password(KEYRING_SERVICE, key)
    except Exception:
        pass


# ── read ───────────────────────────────────────────────────────────────

def get_username() -> str | None:
    return _keyring_get(KEYRING_USERNAME_KEY)


def get_password() -> str | None:
    return _keyring_get(KEYRING_PASSWORD_KEY)


def has_credentials() -> bool:
    return bool(get_username())


def get_login_method() -> str | None:
    """How the stored account signs in: "web", "qr", "password" (or None)."""
    method = _keyring_get(KEYRING_LOGIN_METHOD_KEY)
    if method:
        return method
    if get_username() and get_password():
        return "password"
    return None


def get_steamid() -> str | None:
    return _keyring_get(KEYRING_STEAMID_KEY)


def get_login_secure() -> str | None:
    return _keyring_get(KEYRING_LOGIN_SECURE_KEY)


def get_steam_web_api_key() -> str | None:
    return _keyring_get(KEYRING_STEAM_API_KEY)


def set_steam_web_api_key(key: str) -> None:
    _keyring_set(KEYRING_STEAM_API_KEY, key)


def clear_steam_web_api_key() -> None:
    _keyring_delete(KEYRING_STEAM_API_KEY)


def has_api_key() -> bool:
    return bool(get_steam_web_api_key())


# ── write ──────────────────────────────────────────────────────────────

def set_credentials(username: str, password: str) -> None:
    _keyring_set(KEYRING_USERNAME_KEY, username)
    _keyring_set(KEYRING_PASSWORD_KEY, password)
    _keyring_set(KEYRING_LOGIN_METHOD_KEY, "password")


def set_web_username(username: str) -> None:
    """Store an account signed in via Steam's web login page (the session
    itself lives in the webview's persistent cookie store)."""
    _keyring_set(KEYRING_USERNAME_KEY, username)
    _keyring_set(KEYRING_LOGIN_METHOD_KEY, "web")
    _keyring_delete(KEYRING_PASSWORD_KEY)


def set_steam_session(
    username: str,
    method: str,
    password: str | None = None,
    steamid: str | None = None,
    login_secure: str | None = None,
) -> None:
    """Store a signed-in Steam session.

    ``method`` is "qr" or "password".  The password is only kept for the
    password flow (handed to DepotDownloader for downloads); the web session
    is represented by the ``steamLoginSecure`` cookie value.
    """
    _keyring_set(KEYRING_USERNAME_KEY, username)
    _keyring_set(KEYRING_LOGIN_METHOD_KEY, method)
    if password:
        _keyring_set(KEYRING_PASSWORD_KEY, password)
    else:
        _keyring_delete(KEYRING_PASSWORD_KEY)
    for key, value in ((KEYRING_STEAMID_KEY, steamid), (KEYRING_LOGIN_SECURE_KEY, login_secure)):
        if value:
            _keyring_set(key, value)
        else:
            _keyring_delete(key)


# ── gog token ──────────────────────────────────────────────────────────

def get_gog_token() -> str | None:
    return _keyring_get(KEYRING_GOG_TOKEN_KEY)


def set_gog_token(token: str) -> None:
    _keyring_set(KEYRING_GOG_TOKEN_KEY, token)


def clear_gog_token() -> None:
    _keyring_delete(KEYRING_GOG_TOKEN_KEY)


# ── delete ─────────────────────────────────────────────────────────────

def clear_credentials() -> None:
    for key in (
        KEYRING_USERNAME_KEY,
        KEYRING_PASSWORD_KEY,
        KEYRING_LOGIN_METHOD_KEY,
        KEYRING_STEAMID_KEY,
        KEYRING_LOGIN_SECURE_KEY,
    ):
        _keyring_delete(key)


# ── helpers ────────────────────────────────────────────────────────────

def steamid_from_cookie(value: str) -> str | None:
    """Extract the SteamID from a ``steamLoginSecure`` cookie value.

    Steam's web session cookie is ``<steamid>||<digest>``.
    """
    if not value:
        return None
    first = value.split("||", 1)[0]
    return first if first.isdigit() else None


def account_name_from_steamid(steamid: str) -> str | None:
    """Resolve a SteamID to the account's persona name via the public
    ``steamcommunity.com`` profile XML (no auth or API key required)."""
    from forager.utils.network import http_get

    url = f"https://steamcommunity.com/profiles/{steamid}/?xml=1"
    try:
        data = http_get(url).decode("utf-8", "replace")
    except Exception:
        return None
    m = re.search(r"<steamID>(.*?)</steamID>", data, re.S)
    if not m:
        return None
    name = html.unescape(re.sub(r"<[^>]+>", "", m.group(1))).strip()
    return name or None
