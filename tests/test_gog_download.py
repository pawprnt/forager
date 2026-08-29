import json
import urllib.request

import pytest

pytest.importorskip("PySide6")

from forager.providers.gog import provider as gog_provider


class _FakeResp:
    def __init__(self, data: bytes, length: int):
        self._data = data
        self.length = length
        self._i = 0

    def read(self, n=-1):
        if n == -1:
            out, self._i = self._data, len(self._data)
            return out
        out = self._data[self._i:self._i + n]
        self._i += len(out)
        return out

    def __enter__(self):
        return self

    def __exit__(self, *a):
        return False


_DOWNLINK = "https://cdn.example.com/installer.bin?token=1"


def test_download_writes_file_into_destination(monkeypatch, tmp_path):
    monkeypatch.setattr(gog_provider, "get_gog_token", lambda: "TOKEN")
    payload = json.dumps({
        "downloads": {
            "products": [
                {"downloads": [{"os": "windows", "files": [{"downlink": _DOWNLINK}]}]}
            ]
        }
    }).encode()

    progress = []

    def fake_urlopen(req, timeout=0):
        url = req.full_url if hasattr(req, "full_url") else str(req)
        if "downloads" in url:
            return _FakeResp(payload, len(payload))
        return _FakeResp(b"INSTALLER-BYTES", 15)

    monkeypatch.setattr(urllib.request, "urlopen", fake_urlopen)
    gog_provider.GogProvider().download("123", str(tmp_path), on_progress=progress.append)

    files = list(tmp_path.iterdir())
    assert any(f.name == "installer.bin" for f in files)
    assert (tmp_path / "installer.bin").read_bytes() == b"INSTALLER-BYTES"
    assert progress and progress[-1].percent == 100.0
    assert any(p.speed > 0 for p in progress)
