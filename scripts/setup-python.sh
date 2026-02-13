#!/usr/bin/env bash
set -euo pipefail

# Create a local virtual environment and install Python deps
REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")"/.. && pwd)"
cd "$REPO_ROOT"

if [[ ! -d .venv ]]; then
  python3 -m venv .venv
fi

# Ensure pip is recent
"$REPO_ROOT/.venv/bin/python" -m pip install --upgrade pip

# Install dependencies
if [[ -f "$REPO_ROOT/requirements.txt" ]]; then
  "$REPO_ROOT/.venv/bin/pip" install -r "$REPO_ROOT/requirements.txt"
else
  echo "requirements.txt not found at $REPO_ROOT" >&2
  exit 1
fi

echo "✔ Python venv ready at $REPO_ROOT/.venv"
echo "To use hvcc:"
echo "  source $REPO_ROOT/.venv/bin/activate && hvcc --help"
