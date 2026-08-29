"""Steam provider (roadmap items 2-3)."""
from __future__ import annotations

from forager.providers.base import (
    Provider,
    register_provider,
    BackendNotConfigured,
    OwnedGame,
)
from forager.providers.steam import credentials, library, downloader


@register_provider
class SteamProvider(Provider):
    name = "steam"

    def is_configured(self) -> bool:
        return bool(credentials.has_credentials() or credentials.has_api_key())

    def list_owned(self, account=None) -> list[OwnedGame]:
        owned = library.owned_games()
        installed_ids: set[str] = set()
        try:
            from forager.library.scanner import _scan_steam

            installed_ids = {g.app_id for g in _scan_steam() if g.app_id}
        except Exception:
            installed_ids = set()
        return [
            OwnedGame(
                app_id=g["appid"],
                name=g["name"],
                provider="steam",
                installed=g["appid"] in installed_ids,
            )
            for g in owned
        ]

    def download(self, app_id, destination, on_progress=None, cancel=None) -> None:
        if not credentials.has_credentials():
            raise BackendNotConfigured("Sign in to Steam to download games.")
        downloader.download_app(
            app_id, destination, on_progress=on_progress, cancel=cancel
        )
