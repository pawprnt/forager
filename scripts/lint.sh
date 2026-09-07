#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

echo "=== Ruff check ==="
ruff check "$ROOT/src" "$ROOT/scripts" && echo "OK" || exit 1

echo ""
echo "=== Ruff format ==="
ruff format --check "$ROOT/src" "$ROOT/scripts" && echo "OK" || exit 1
