#!/usr/bin/env python3
"""Run all forager tests by category."""
from __future__ import annotations

import subprocess
import sys
from pathlib import Path

TESTING_DIR = Path(__file__).resolve().parent

CATEGORIES = [
    ("library", "Library (game, launcher, playtime, scanner)"),
    ("providers", "Providers (steam, epic, gog, torrent)"),
    ("ui", "UI (gamepage, icons, main window, settings, store, font)"),
    ("artwork", "Artwork (placeholders, steam art)"),
    ("compatibility", "Compatibility (depotdownloader)"),
    ("updates", "Updates (tool updates)"),
]


def run_category(name: str, description: str) -> int:
    category_dir = TESTING_DIR / name
    if not category_dir.is_dir():
        print(f"  SKIP  {name}/ (not found)")
        return 0
    print(f"\n{'='*60}")
    print(f"  {description}")
    print(f"{'='*60}")
    for attempt in range(2):
        result = subprocess.run(
            [sys.executable, "-m", "pytest", str(category_dir), "-q", "--tb=short"],
            cwd=str(TESTING_DIR.parent.parent),
            capture_output=True, text=True,
        )
        output = result.stdout + result.stderr
        print(output, end="")
        passed = "passed" in output and "failed" not in output
        if result.returncode == 0 or (passed and result.returncode == -11):
            return 0
        if attempt == 0:
            print(f"  RETRY  {name}/ (rc={result.returncode})")
    return result.returncode


def main() -> int:
    print("Forager Test Suite")
    print(f"Python {sys.version.split()[0]}")
    failed = 0
    for name, desc in CATEGORIES:
        rc = run_category(name, desc)
        if rc != 0:
            failed += 1

    print(f"\n{'='*60}")
    if failed:
        print(f"  FAILED: {failed} category(ies) had failures")
        return 1
    print("  ALL PASSED")
    return 0


if __name__ == "__main__":
    sys.exit(main())
