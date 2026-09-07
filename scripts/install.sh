#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

echo "Installing forager in development mode..."
pip install -e "$ROOT" "$@"
echo "Done. Run with: scripts/run.sh"
