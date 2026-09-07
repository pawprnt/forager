#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
export PYTHONPATH="${ROOT}/src:${PYTHONPATH:-}"
export QT_QPA_PLATFORM="${QT_QPA_PLATFORM:-offscreen}"
export QTWEBENGINE_DISABLE_SANDBOX="${QTWEBENGINE_DISABLE_SANDBOX:-1}"

# Use venv python if available, else system python3
if [[ -x "${ROOT}/../envs/forager-venv/bin/python3" ]]; then
    PYTHON="${ROOT}/../envs/forager-venv/bin/python3"
else
    PYTHON="$(command -v python3)"
fi

PYTEST_ARGS=("${ROOT}/scripts/testing/" "-q" "--tb=short")
if [[ $# -gt 0 ]]; then
    PYTEST_ARGS=("$@")
fi

# Use xvfb-run if available (needed for QtWebEngine tests)
if command -v xvfb-run &>/dev/null; then
    exec xvfb-run -a "$PYTHON" -m pytest "${PYTEST_ARGS[@]}"
else
    exec "$PYTHON" -m pytest "${PYTEST_ARGS[@]}"
fi
