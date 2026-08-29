"""Store-backend providers.

Importing the subpackages registers each backend with
``forager.providers.base.register_provider``. Backends that require optional
binaries import those lazily so a missing dependency never breaks import.
"""
from __future__ import annotations

from forager.providers.base import (  # noqa: F401
    Provider,
    ProviderError,
    BackendNotConfigured,
    OwnedGame,
    ProgressFn,
    register_provider,
    get_provider,
    available_providers,
    PROVIDERS,
)

try:
    from forager.providers import steam  # noqa: F401  (registers Steam)
except Exception:  # pragma: no cover - steam is always present
    pass
try:
    from forager.providers import epic  # noqa: F401
except Exception:
    pass
try:
    from forager.providers import gog  # noqa: F401
except Exception:
    pass
try:
    from forager.providers import torrent  # noqa: F401
except Exception:
    pass
