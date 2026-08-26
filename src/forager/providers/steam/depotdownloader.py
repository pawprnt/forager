"""DepotDownloader subprocess management for Steam login verification.

Handles running DepotDownloader to validate credentials and sessions,
including Steam Guard code interaction and output parsing.
"""
from __future__ import annotations

import os
import select
import shutil
import subprocess
import tempfile
import threading
from pathlib import Path
from typing import Callable

from forager.compatibility.proton import (
    PROTON_APPID,
    PROTON_DEPOTS,
    DEPOTDL_DIR,
    depotdownloader_bin,
    ensure_depotdownloader,
)

LOGIN_TIMEOUT = 180.0

_GUARD_MARKERS = (
    "steam guard",
    "2 factor auth code",
    "authentication code sent to your email",
)

_SESSION_PROMPT_MARKER = "enter account password for"
_TOKEN_REJECTED_MARKER = "access token was rejected"


def clear_session() -> None:
    """Drop DepotDownloader's stored sessions (refresh tokens / sentry data)."""
    try:
        cfg = DEPOTDL_DIR / "account.config"
        if cfg.is_file():
            cfg.unlink()
    except OSError:
        pass


def _login_cmd(username: str, password: str, remember: bool, download_dir: Path) -> list[str]:
    cmd = [
        str(depotdownloader_bin()),
        "-app", PROTON_APPID,
        "-depot", PROTON_DEPOTS[0],
        "-manifest-only",
        "-dir", str(download_dir),
        "-username", username,
        "-password", password,
    ]
    if remember:
        cmd.append("-remember-password")
    return cmd


def _session_cmd(username: str, download_dir: Path) -> list[str]:
    """Reuse a stored refresh token: no password, no Steam Guard."""
    return [
        str(depotdownloader_bin()),
        "-app", PROTON_APPID,
        "-depot", PROTON_DEPOTS[0],
        "-manifest-only",
        "-dir", str(download_dir),
        "-username", username,
        "-remember-password",
    ]


def _run_dd(
    cmd: list[str],
    timeout: float,
    cancel_event: threading.Event | None = None,
    on_line: Callable[[str], None] | None = None,
) -> tuple[list[str], str, int, bool]:
    """Run DepotDownloader from DEPOTDL_DIR.

    Calls ``on_line(line)`` for every completed line of combined output.
    Returns ``(log, tail, returncode, cancelled)`` where ``tail`` is the
    trailing line fragment (prompts are written without a newline).
    """
    ensure_depotdownloader()
    proc = subprocess.Popen(
        cmd,
        cwd=str(DEPOTDL_DIR),
        stdin=subprocess.PIPE,
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        text=True,
        bufsize=1,
    )
    assert proc.stdout is not None
    fd = proc.stdout.fileno()
    buf = ""
    log: list[str] = []
    cancelled = False
    deadline = threading.Event()
    deadline_timer = threading.Timer(timeout, deadline.set)
    deadline_timer.start()
    try:
        while proc.poll() is None:
            if deadline.is_set() or (cancel_event is not None and cancel_event.is_set()):
                proc.terminate()
                cancelled = True
                break
            r, _, _ = select.select([fd], [], [], 0.5)
            if not r:
                continue
            data = os.read(fd, 4096)
            if not data:
                break
            buf += data.decode("utf-8", errors="replace")
            while "\n" in buf:
                line, buf = buf.split("\n", 1)
                line = line.rstrip("\r")
                log.append(line)
                if len(log) > 200:
                    log = log[-100:]
                if on_line is not None:
                    on_line(line)
        try:
            proc.wait(timeout=10)
        except subprocess.TimeoutExpired:
            proc.kill()
            proc.wait()
    finally:
        deadline_timer.cancel()
    return log, buf, proc.returncode, cancelled


def verify_login(
    username: str,
    password: str,
    remember: bool,
    guard_prompt: Callable[[str], str | None],
    cancel_event: threading.Event | None = None,
) -> tuple[bool, str]:
    """Validate Steam credentials with a manifest-only depot download.

    ``guard_prompt(message)`` is called from this thread whenever Steam Guard
    asks for an auth/email code; it must return the code (or None to cancel).
    Runs DepotDownloader from DEPOTDL_DIR so its ``account.config`` (refresh
    tokens / sentry data) persists across runs.
    """
    ensure_depotdownloader()
    scratch = Path(tempfile.mkdtemp(prefix="forager-login-"))
    proc: subprocess.Popen | None = None
    try:
        proc = subprocess.Popen(
            _login_cmd(username, password, remember, scratch),
            cwd=str(DEPOTDL_DIR),
            stdin=subprocess.PIPE,
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            text=True,
            bufsize=1,
        )
        assert proc.stdout is not None and proc.stdin is not None
        fd = proc.stdout.fileno()
        buf = ""
        log: list[str] = []
        cancelled = False
        deadline = threading.Event()
        deadline_timer = threading.Timer(LOGIN_TIMEOUT, deadline.set)
        deadline_timer.start()
        try:
            while proc.poll() is None:
                if deadline.is_set():
                    proc.terminate()
                    break
                if cancel_event is not None and cancel_event.is_set():
                    proc.terminate()
                    break
                r, _, _ = select.select([fd], [], [], 0.5)
                if not r:
                    continue
                data = os.read(fd, 4096)
                if not data:
                    break
                buf += data.decode("utf-8", errors="replace")
                while "\n" in buf:
                    line, buf = buf.split("\n", 1)
                    log.append(line.rstrip())
                    if len(log) > 200:
                        log = log[-100:]
                lower = buf.lower()
                if any(marker in lower for marker in _GUARD_MARKERS):
                    prompt = buf.strip() or "Steam Guard authentication required"
                    code = guard_prompt(prompt)
                    if not code:
                        cancelled = True
                        try:
                            proc.stdin.close()
                        except Exception:
                            pass
                        break
                    try:
                        proc.stdin.write(code + "\n")
                        proc.stdin.flush()
                    except (BrokenPipeError, OSError):
                        break
                    buf = ""
        finally:
            deadline_timer.cancel()

        try:
            proc.wait(timeout=10)
        except subprocess.TimeoutExpired:
            proc.kill()
            proc.wait()
        returncode = proc.returncode
        tail = "\n".join(log[-20:]).lower()
        if cancelled or (cancel_event is not None and cancel_event.is_set()):
            return False, "Sign-in cancelled"
    finally:
        if proc is not None and proc.poll() is None:
            proc.kill()
        shutil.rmtree(scratch, ignore_errors=True)

    if returncode == 0 and "unable to get steam3 credentials" not in tail:
        return True, f"Signed in as {username}"
    return False, tail or f"DepotDownloader exited with code {returncode}"


def verify_session(
    username: str,
    cancel_event: threading.Event | None = None,
) -> tuple[bool, str]:
    """Validate a previously stored session (refresh token) without a password.

    Returns ``(True, "Signed in as X")`` when DepotDownloader can reuse the
    cached token, or ``(False, reason)`` if the session is gone/rejected.
    """
    scratch = Path(tempfile.mkdtemp(prefix="forager-session-"))
    try:
        log, tail, returncode, cancelled = _run_dd(
            _session_cmd(username, scratch), LOGIN_TIMEOUT, cancel_event
        )
    finally:
        shutil.rmtree(scratch, ignore_errors=True)

    if cancelled:
        return False, "Check cancelled"
    combined = "\n".join(log[-20:]) + "\n" + tail
    low = combined.lower()
    if _SESSION_PROMPT_MARKER in low:
        return False, "No stored session for this account; sign in again (Steam login or password)."
    if _TOKEN_REJECTED_MARKER in low:
        return False, "Stored session was rejected; sign in again (Steam login or password)."
    if returncode == 0 and "unable to get steam3 credentials" not in low:
        return True, f"Signed in as {username}"
    return False, low or f"DepotDownloader exited with code {returncode}"
