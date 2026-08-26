"""Steam account management — re-exports for backward compatibility.

The implementation has been split into two focused modules:

- ``credentials`` — keyring-based credential storage (usernames, passwords,
  login methods, SteamIDs, login-secure cookies).
- ``depotdownloader`` — DepotDownloader subprocess management (login
  verification, session validation, Steam Guard interaction).

All public names remain importable from this module.
"""
from __future__ import annotations

# ── credential management (from credentials.py) ────────────────────────
from forager.providers.steam.credentials import (  # noqa: F401
    KEYRING_USERNAME_KEY,
    KEYRING_PASSWORD_KEY,
    KEYRING_LOGIN_METHOD_KEY,
    KEYRING_STEAMID_KEY,
    KEYRING_LOGIN_SECURE_KEY,
    get_username,
    get_password,
    has_credentials,
    get_login_method,
    set_credentials,
    set_web_username,
    get_steamid,
    get_login_secure,
    set_steam_session,
    steamid_from_cookie,
    account_name_from_steamid,
    clear_credentials,
)

# ── DepotDownloader subprocess management (from depotdownloader.py) ────
from forager.providers.steam.depotdownloader import (  # noqa: F401
    LOGIN_TIMEOUT,
    clear_session,
    verify_login,
    verify_session,
)
