"""GOG backend using GOG's unofficial web API.

Authenticates with a bearer token (stored via the shared keyring helpers in
``forager.providers.steam.credentials``) and serves owned games plus offline
installer downloads through the common Provider interface.
"""
from __future__ import annotations

import json
import urllib.request
from pathlib import Path
from typing import Callable, Optional

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

PRODUCTS_URL = "https://embed.gog.com/account/getFilteredProducts?mediaType=1&page=1"
DOWNLOADS_URL = "https://api.gog.com/products/{app_id}/downloads"


@register_provider
class GogProvider(Provider):
    name = "gog"

    def is_configured(self) -> bool:
        token = get_gog_token()
        return bool(token and token.strip())

    def list_owned(self, account: Optional[str] = None) -> list[OwnedGame]:
        token = get_gog_token()
        if not token:
            return []
        try:
            body = http_get(PRODUCTS_URL)
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
        on_progress: Optional[ProgressFn] = None,
        cancel: Optional[object] = None,
    ) -> None:
        token = get_gog_token()
        if not token:
            raise BackendNotConfigured("GOG token not configured")

        dest = Path(destination)
        auth_header = {"Authorization": f"Bearer {token}"}

        try:
            url = DOWNLOADS_URL.format(app_id=app_id)
            req = urllib.request.Request(url, headers=auth_header)
            with urllib.request.urlopen(req, timeout=30) as resp:
                data = json.loads(resp.read().decode("utf-8"))
        except Exception as exc:
            raise ProviderError(f"Failed to fetch GOG downloads: {exc}")

        downlink = self._pick_downlink(data)
        if not downlink:
            raise ProviderError("No suitable Windows installer found for this GOG product")

        try:
            req = urllib.request.Request(downlink, headers=auth_header)
            with urllib.request.urlopen(req, timeout=60) as resp:
                total = resp.length if resp.length is not None else 0
                downloaded = 0
                chunk_size = 1 << 16
                last_t = None
                speed = 0
                with open(dest, "wb") as out:
                    while True:
                        if cancel is not None and getattr(cancel, "is_set", lambda: False)():
                            raise ProviderError("Download cancelled")
                        chunk = resp.read(chunk_size)
                        if not chunk:
                            break
                        out.write(chunk)
                        downloaded += len(chunk)
                        percent = (downloaded / total * 100) if total else 0.0
                        if on_progress is not None:
                            on_progress(
                                DownloadProgress("download", percent, downloaded, total, speed)
                            )
        except ProviderError:
            raise
        except Exception as exc:
            raise ProviderError(f"GOG download failed: {exc}")

    @staticmethod
    def _pick_downlink(data: dict) -> Optional[str]:
        for item in data.get("downloads", {}).get("products", []) or []:
            for platform in item.get("downloads", []) or []:
                if platform.get("os") != "windows":
                    continue
                for entry in platform.get("files", []) or []:
                    link = entry.get("downlink")
                    if link:
                        return link
        return None
