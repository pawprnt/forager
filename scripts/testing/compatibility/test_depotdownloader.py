import pytest

from forager.compatibility import proton


def test_flatten_depotdownloader(tmp_path, monkeypatch):
    monkeypatch.setattr(proton, "DEPOTDL_DIR", tmp_path)
    nested = tmp_path / "DepotDownloader-linux-x64"
    nested.mkdir()
    (nested / "DepotDownloader").write_bytes(b"#!/bin/sh\n")
    (nested / "DepotDownloader.runtimeconfig.json").write_text("{}")
    proton._flatten_depotdownloader()
    assert (tmp_path / "DepotDownloader").is_file()
    assert (tmp_path / "DepotDownloader.runtimeconfig.json").is_file()
    assert not nested.exists()
    assert proton.depotdownloader_bin().name == "DepotDownloader"
