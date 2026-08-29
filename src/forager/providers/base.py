"""Provider abstraction shared by every store backend (Steam, Epic, GOG, torrent).

Each backend exposes a uniform surface so the library, downloads, and store
pages can treat them interchangeably. See ``docs/architecture/providers.md``.
"""
from __future__ import annotations

from abc import ABC, abstractmethod
from dataclasses import dataclass
from pathlib import Path
from typing import Callable, Optional

from forager.compatibility.proton import DownloadProgress


class ProviderError(Exception):
    """Base class for provider failures."""


class BackendNotConfigured(ProviderError):
    """Raised when a backend's credentials/binary are missing."""


@dataclass
class OwnedGame:
    app_id: str
    name: str
    provider: str
    installed: bool = False
    icon_url: Optional[str] = None


ProgressFn = Callable[[DownloadProgress], None]


class Provider(ABC):
    #: Unique backend name (also the registry key).
    name: str

    @abstractmethod
    def is_configured(self) -> bool:
        """True when the backend has credentials/binary available."""

    @abstractmethod
    def list_owned(self, account: Optional[str] = None) -> list[OwnedGame]:
        """Return games the active account owns on this backend."""

    @abstractmethod
    def download(
        self,
        app_id: str,
        destination: str | Path,
        on_progress: Optional[ProgressFn] = None,
        cancel: Optional[object] = None,
    ) -> None:
        """Download *app_id* to *destination*, emitting DownloadProgress."""


PROVIDERS: dict[str, type["Provider"]] = {}


def register_provider(cls: type[Provider]) -> type[Provider]:
    PROVIDERS[cls.name] = cls
    return cls


def get_provider(name: str) -> Provider:
    if name not in PROVIDERS:
        raise ProviderError(f"Unknown provider: {name}")
    return PROVIDERS[name]()


def available_providers() -> list[str]:
    return sorted(PROVIDERS)
