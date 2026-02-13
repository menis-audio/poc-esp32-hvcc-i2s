#!/usr/bin/env bash
set -euo pipefail

REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")"/.. && pwd)"

if [[ ! -x "$REPO_ROOT/.venv/bin/hvcc" ]]; then
  echo "Bootstrapping hvcc in repo-local venv..."
  "$REPO_ROOT/scripts/setup-python.sh"
fi

exec "$REPO_ROOT/.venv/bin/hvcc" "$@"
