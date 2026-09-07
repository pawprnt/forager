"""GOG backend using GOG's unofficial web API.

Authenticates with a bearer token (stored via the shared keyring helpers in
``forager.providers.steam.credentials``) and serves owned games plus offline
installer downloads through the common Provider interface.
"""
from __future__ import annotations

import json
from pathlib import Path

from forager.compatibility.proton import DownloadProgress
from forager.providers.base import (
    BackendNotConfigured,
    OwnedGame,
    ProgressFn,
    Provider,
    ProviderError,
    register_provider,
)
from forager.providers.steam.credentials import get_gog_token
from forager.utils.network import http_get
from forager.utils.download import download_to_file

PRODUCTS_URL = "https://embed.gog.com/account/getFilteredProducts?mediaType=1&page=1"
DOWNLOADS_URL = "https://api.gog.com/products/{app_id}/downloads"


@register_provider
class GogProvider(Provider):
    name = "gog"

    def is_configured(self) -> bool:
        token = get_gog_token()
        return bool(token and token.strip())

    def list_owned(self, account: str | None = None) -> list[OwnedGame]:
        token = get_gog_token()
        if not token:
            return []
        try:
            body = http_get(PRODUCTS_URL, headers={"Authorization": f"Bearer {token}"})
            data = json.loads(body.decode("utf-8"))
            products = data.get("products") or []
            games: list[OwnedGame] = []
            for product in products:
                pid = product.get("id")
                title = product.get("title")
                if pid is None or not title:
                    continue
                games.append(
                    OwnedGame(
                        app_id=str(pid),
                        name=title,
                        provider="gog",
                        installed=False,
                    )
                )
            return games
        except Exception:
            return []

    def download(
        self,
        app_id: str,
        destination: str | Path,
        on_progress: ProgressFn | None = None,
        cancel: object | None = None,
    ) -> None:
        token = get_gog_token()
        if not token:
            raise BackendNotConfigured("GOG token not configured")

        dest = Path(destination)
        auth_header = {"Authorization": f"Bearer {token}"}

        try:
            url = DOWNLOADS_URL.format(app_id=app_id)
            raw = http_get(url, timeout=30, headers=auth_header)
            data = json.loads(raw.decode("utf-8"))
        except Exception as exc:
            raise ProviderError(f"Failed to fetch GOG downloads: {exc}")

        downlink = self._pick_downlink(data)
        if not downlink:
            raise ProviderError("No suitable Windows installer found for this GOG product")

        filename = Path(downlink.split("?")[0]).name or f"gog-{app_id}.bin"
        dest.mkdir(parents=True, exist_ok=True)
        out_path = dest / filename

        try:
            download_to_file(downlink, out_path, timeout=60, on_progress=on_progress, cancel=cancel)
        except Exception as exc:
            out_path.unlink(missing_ok=True)
            raise ProviderError(f"GOG download failed: {exc}")

    @staticmethod
    def _pick_downlink(data: dict) -> str | None:
        for item in data.get("downloads", {}).get("products", []) or []:
            for platform in item.get("downloads", []) or []:
                if platform.get("os") != "windows":
                    continue
                for entry in platform.get("files", []) or []:
                    link = entry.get("downlink")
                    if link:
                        return link
        return None
