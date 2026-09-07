"""Shared download and archive extraction helpers.

Used by the Proton updater, DepotDownloader provisioner, and tool-update
downloader to avoid duplicating the download-stream-extract loop.
"""
from __future__ import annotations

import shutil
import tempfile
import time
import urllib.request
import zipfile
import tarfile
from pathlib import Path
from typing import Callable

from forager.compatibility.proton import DownloadProgress


def download_to_file(
    url: str,
    dest: Path,
    *,
    timeout: float = 120.0,
    on_progress: Callable[[DownloadProgress], None] | None = None,
    cancel: object | None = None,
) -> None:
    """Stream *url* into *dest* with optional progress reporting."""
    dest.parent.mkdir(parents=True, exist_ok=True)
    req = urllib.request.Request(url, headers={"User-Agent": "forager"})
    with urllib.request.urlopen(req, timeout=timeout) as resp:
        total = getattr(resp, "length", None) or 0
        downloaded = 0
        chunk_size = 1 << 16
        speed = 0
        last_done = 0
        last_t = time.monotonic()
        socket_obj = getattr(resp, "fp", None)
        if socket_obj is not None and hasattr(socket_obj, "raw"):
            try:
                socket_obj.raw.settimeout(timeout)
            except Exception:
                pass
        with open(dest, "wb") as out:
            while True:
                if cancel is not None and getattr(cancel, "is_set", lambda: False)():
                    dest.unlink(missing_ok=True)
                    return
                chunk = resp.read(chunk_size)
                if not chunk:
                    break
                out.write(chunk)
                downloaded += len(chunk)
                now = time.monotonic()
                dt = now - last_t
                if dt > 0:
                    speed = int((downloaded - last_done) / dt)
                last_done, last_t = downloaded, now
                if on_progress is not None:
                    on_progress(DownloadProgress(
                        stage="download",
                        percent=(downloaded / total * 100) if total else 0.0,
                        done=downloaded,
                        total=total,
                        speed=speed,
                    ))


def _safe_extract_zip(zf: zipfile.ZipFile, dest: Path) -> None:
    """Extract *zf* into *dest*, rejecting any member that escapes *dest*."""
    dest = dest.resolve()
    for member in zf.infolist():
        target = (dest / member.filename).resolve()
        if not (target == dest or str(target).startswith(str(dest) + "/")):
            raise RuntimeError(f"Zip path traversal attempt: {member.filename}")
    zf.extractall(dest)


def _safe_extract_tar(tf: tarfile.TarFile, dest: Path) -> None:
    """Extract *tf* into *dest*, rejecting any member that escapes *dest*."""
    dest = dest.resolve()
    for member in tf.getmembers():
        target = (dest / member.name).resolve()
        if not (target == dest or str(target).startswith(str(dest) + "/")):
            raise RuntimeError(f"Tar path traversal attempt: {member.name}")
        if member.issym() or member.islnk():
            link_target = (dest / member.linkname).resolve() if not member.linkname.startswith("/") else Path(member.linkname).resolve()
            if member.issym() and not (link_target == dest or str(link_target).startswith(str(dest) + "/")):
                raise RuntimeError(f"Tar symlink traversal attempt: {member.name} -> {member.linkname}")
    tf.extractall(dest)


def download_and_extract_zip(url: str, dest: Path, *, timeout: float = 120.0) -> None:
    """Download *url* to a temp file and extract the zip into *dest*."""
    dest.mkdir(parents=True, exist_ok=True)
    with tempfile.NamedTemporaryFile(suffix=".zip", dir=dest.parent) as tmp:
        download_to_file(url, Path(tmp.name), timeout=timeout)
        with zipfile.ZipFile(tmp.name) as zf:
            _safe_extract_zip(zf, dest)


def download_and_extract_tar(url: str, dest: Path, *, timeout: float = 120.0) -> None:
    """Download *url* to a temp file and extract the tarball into *dest*."""
    dest.mkdir(parents=True, exist_ok=True)
    suffix = ".tar.gz" if url.endswith((".gz", ".tgz")) else ".tar"
    with tempfile.NamedTemporaryFile(suffix=suffix, dir=dest.parent) as tmp:
        download_to_file(url, Path(tmp.name), timeout=timeout)
        with tarfile.open(tmp.name) as tf:
            _safe_extract_tar(tf, dest)
