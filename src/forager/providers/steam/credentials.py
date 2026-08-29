"""Steam credential management via the system keyring.

Handles storing, retrieving, and clearing Steam usernames, passwords,
login methods, SteamIDs, and login-secure cookies.  No network or
subprocess work lives here — that belongs in ``depotdownloader``.
"""
from __future__ import annotations

import html
import re
import urllib.request

from forager.core.constants import KEYRING_SERVICE
from forager.utils.network import USER_AGENT

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


# ── read ───────────────────────────────────────────────────────────────

def get_username() -> str | None:
    if _keyring is not None:
        try:
            stored = _keyring.get_password(KEYRING_SERVICE, KEYRING_USERNAME_KEY)
            if stored:
                return stored
        except Exception:
            pass
    return None


def get_password() -> str | None:
    if _keyring is not None:
        try:
            stored = _keyring.get_password(KEYRING_SERVICE, KEYRING_PASSWORD_KEY)
            if stored:
                return stored
        except Exception:
            pass
    return None


def has_credentials() -> bool:
    return bool(get_username())


def get_login_method() -> str | None:
    """How the stored account signs in: "web", "qr", "password" (or None)."""
    if _keyring is not None:
        try:
            method = _keyring.get_password(KEYRING_SERVICE, KEYRING_LOGIN_METHOD_KEY)
            if method:
                return method
        except Exception:
            pass
    if get_username() and get_password():
        return "password"
    return None


def get_steamid() -> str | None:
    if _keyring is not None:
        try:
            stored = _keyring.get_password(KEYRING_SERVICE, KEYRING_STEAMID_KEY)
            if stored:
                return stored
        except Exception:
            pass
    return None


def get_login_secure() -> str | None:
    if _keyring is not None:
        try:
            stored = _keyring.get_password(KEYRING_SERVICE, KEYRING_LOGIN_SECURE_KEY)
            if stored:
                return stored
        except Exception:
            pass
    return None


def get_steam_web_api_key() -> str | None:
    if _keyring is not None:
        try:
            stored = _keyring.get_password(KEYRING_SERVICE, KEYRING_STEAM_API_KEY)
            if stored:
                return stored
        except Exception:
            pass
    return None


def set_steam_web_api_key(key: str) -> None:
    if _keyring is None:
        raise RuntimeError("keyring backend unavailable")
    _keyring.set_password(KEYRING_SERVICE, KEYRING_STEAM_API_KEY, key)


def clear_steam_web_api_key() -> None:
    if _keyring is None:
        return
    try:
        _keyring.delete_password(KEYRING_SERVICE, KEYRING_STEAM_API_KEY)
    except Exception:
        pass


def has_api_key() -> bool:
    return bool(get_steam_web_api_key())


# ── write ──────────────────────────────────────────────────────────────

def set_credentials(username: str, password: str) -> None:
    if _keyring is None:
        raise RuntimeError("keyring backend unavailable")
    _keyring.set_password(KEYRING_SERVICE, KEYRING_USERNAME_KEY, username)
    _keyring.set_password(KEYRING_SERVICE, KEYRING_PASSWORD_KEY, password)
    _keyring.set_password(KEYRING_SERVICE, KEYRING_LOGIN_METHOD_KEY, "password")


def set_web_username(username: str) -> None:
    """Store an account signed in via Steam's web login page (the session
    itself lives in the webview's persistent cookie store)."""
    if _keyring is None:
        raise RuntimeError("keyring backend unavailable")
    _keyring.set_password(KEYRING_SERVICE, KEYRING_USERNAME_KEY, username)
    _keyring.set_password(KEYRING_SERVICE, KEYRING_LOGIN_METHOD_KEY, "web")
    try:
        _keyring.delete_password(KEYRING_SERVICE, KEYRING_PASSWORD_KEY)
    except Exception:
        pass


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
    if _keyring is None:
        raise RuntimeError("keyring backend unavailable")
    _keyring.set_password(KEYRING_SERVICE, KEYRING_USERNAME_KEY, username)
    _keyring.set_password(KEYRING_SERVICE, KEYRING_LOGIN_METHOD_KEY, method)
    if password:
        _keyring.set_password(KEYRING_SERVICE, KEYRING_PASSWORD_KEY, password)
    else:
        try:
            _keyring.delete_password(KEYRING_SERVICE, KEYRING_PASSWORD_KEY)
        except Exception:
            pass
    for key, value in ((KEYRING_STEAMID_KEY, steamid), (KEYRING_LOGIN_SECURE_KEY, login_secure)):
        if value:
            _keyring.set_password(KEYRING_SERVICE, key, value)
        else:
            try:
                _keyring.delete_password(KEYRING_SERVICE, key)
            except Exception:
                pass


# ── gog token ──────────────────────────────────────────────────────────

def get_gog_token() -> str | None:
    if _keyring is not None:
        try:
            token = _keyring.get_password(KEYRING_SERVICE, KEYRING_GOG_TOKEN_KEY)
            if token:
                return token
        except Exception:
            pass
    return None


def set_gog_token(token: str) -> None:
    if _keyring is None:
        raise RuntimeError("keyring backend unavailable")
    _keyring.set_password(KEYRING_SERVICE, KEYRING_GOG_TOKEN_KEY, token)


def clear_gog_token() -> None:
    if _keyring is None:
        return
    try:
        _keyring.delete_password(KEYRING_SERVICE, KEYRING_GOG_TOKEN_KEY)
    except Exception:
        pass


# ── delete ─────────────────────────────────────────────────────────────

def clear_credentials() -> None:
    if _keyring is None:
        return
    for key in (
        KEYRING_USERNAME_KEY,
        KEYRING_PASSWORD_KEY,
        KEYRING_LOGIN_METHOD_KEY,
        KEYRING_STEAMID_KEY,
        KEYRING_LOGIN_SECURE_KEY,
    ):
        try:
            _keyring.delete_password(KEYRING_SERVICE, key)
        except Exception:
            pass


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
    url = f"https://steamcommunity.com/profiles/{steamid}/?xml=1"
    try:
        req = urllib.request.Request(url, headers={"User-Agent": USER_AGENT})
        with urllib.request.urlopen(req, timeout=15) as resp:
            data = resp.read(1 << 20).decode("utf-8", "replace")
    except Exception:
        return None
    m = re.search(r"<steamID>(.*?)</steamID>", data, re.S)
    if not m:
        return None
    name = html.unescape(re.sub(r"<[^>]+>", "", m.group(1))).strip()
    return name or None
