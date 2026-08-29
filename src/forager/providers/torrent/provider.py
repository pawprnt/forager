"""Torrent backend powered by ``libtorrent`` (roadmap: generic downloader).

This provider is a generic downloader, NOT a library source. Torrents are not
"owned library" entries, so ``list_owned`` always returns ``[]``. The backend
only exposes ``download`` for pulling a magnet link or ``.torrent`` URL into a
destination folder.

``libtorrent`` is imported lazily so that merely importing this module never
fails on a machine without the native extension installed.
"""

from __future__ import annotations

from pathlib import Path
from typing import Optional

from forager.compatibility.proton import DownloadProgress
from forager.providers.base import BackendNotConfigured, Provider, register_provider


@register_provider
class TorrentProvider(Provider):
    name = "torrent"

    def is_configured(self) -> bool:
        # A torrent backend is "configured" iff libtorrent can be imported.
        # Import inside the method so module import never requires libtorrent.
        try:
            import libtorrent  # noqa: F401
        except Exception:
            return False
        return True

    def list_owned(self, account: Optional[str] = None) -> list:
        # Torrents are not an "owned library" — this backend is a generic
        # downloader, not a store/library source. There is nothing to list.
        return []

    def download(
        self,
        uri: str,
        destination: str | Path,
        on_progress: Optional[object] = None,
        cancel: Optional[object] = None,
    ) -> None:
        # Lazily import libtorrent: this method must fail clearly when the
        # native extension is missing rather than breaking module import.
        try:
            import libtorrent as lt
        except Exception as exc:  # pragma: no cover - depends on env
            raise BackendNotConfigured(
                "libtorrent is required for torrent downloads but is not installed"
            ) from exc

        dest = Path(destination)
        dest.mkdir(parents=True, exist_ok=True)

        # Newer libtorrent exposes session_service; older versions use
        # session(). Try the classic constructor first and fall back.
        try:
            ses = lt.session()
        except Exception:  # pragma: no cover - API differences
            ses = lt.session_service()

        params = {
            "save_path": str(dest),
        }

        # Add the torrent from either a magnet URI or an http(s) .torrent URL.
        if uri.startswith("magnet:"):
            try:
                params["url"] = uri
                handle = ses.add_torrent(params)
            except Exception:  # pragma: no cover - API differences
                ses.async_add_torrent(params)
                handle = None
        else:
            # Fetch the .torrent metainfo then add it.
            import urllib.request

            with urllib.request.urlopen(uri, timeout=120) as resp:
                torrent_data = resp.read()
            info = lt.torrent_info(lt.bdecode(torrent_data))
            params["ti"] = info
            handle = ses.add_torrent(params)

        # Resolve the handle if it was added asynchronously.
        if handle is None:
            alert = ses.wait_for_alert(60000)
            while alert is not None:
                if hasattr(alert, "handle") and alert.handle is not None:
                    handle = alert.handle
                    break
                alert = ses.wait_for_alert(60000)
        if handle is None:
            raise BackendNotConfigured("Failed to add torrent")

        # Poll until complete or cancelled.
        while True:
            if cancel is not None and getattr(cancel, "is_set", lambda: False)():
                ses.remove_torrent(handle)
                return

            status = handle.status()
            progress = status.progress or 0.0
            total = getattr(status, "total_wanted", 0) or 0
            done = int(total * progress)
            speed = getattr(status, "download_rate", 0) or 0

            if on_progress is not None:
                on_progress(
                    DownloadProgress(
                        stage="download",
                        percent=progress * 100,
                        done=done,
                        total=total,
                        speed=int(speed),
                    )
                )

            if status.is_seeding or status.progress >= 1.0:
                break

            ses.wait_for_alert(1000)
