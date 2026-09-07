"""Small HTTP helpers shared by the network-touching modules."""
from __future__ import annotations

import urllib.request

from forager.core.constants import VERSION

USER_AGENT = f"forager/{VERSION}"


def http_get(url: str, timeout: float = 15.0, headers: dict[str, str] | None = None) -> bytes:
    """GET *url* with a forager user-agent and return the raw body."""
    all_headers = {"User-Agent": USER_AGENT}
    if headers:
        all_headers.update(headers)
    req = urllib.request.Request(url, headers=all_headers)
    with urllib.request.urlopen(req, timeout=timeout) as resp:
        return resp.read()
