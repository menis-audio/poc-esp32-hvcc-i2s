#!/usr/bin/env bash
set -euo pipefail

echo "[PoC] ESP32 I2S + HVCC build/flash"

# Preflight: ensure required tools are available
command -v hvcc >/dev/null 2>&1 || { echo "Error: hvcc not found on PATH."; echo "Install Heavy (HVCC) and ensure 'hvcc' is available."; exit 1; }
command -v idf.py >/dev/null 2>&1 || { echo "Error: idf.py not found."; echo "Export ESP-IDF (e.g., . \"$HOME/esp/esp-idf/export.sh\")."; exit 1; }

# Optional: set target explicitly for consistency
echo "Setting target to esp32"
idf.py set-target esp32

echo "Regenerating HVCC sources from main/test.pd"
rm -rf main/hvcc
cd main
hvcc test.pd -o hvcc
cd ..

echo "Building firmware"
idf.py build

# Use ESPPORT or PORT if provided, otherwise default
PORT=${ESPPORT:-${PORT:-}}
if [ -n "${PORT}" ]; then
	echo "Flashing to port ${PORT}"
	idf.py -p "${PORT}" flash
else
	echo "Flashing (auto port)"
	idf.py flash
fi

echo "Done. Optionally run: idf.py monitor"
