#!/usr/bin/env bash
set -euo pipefail

echo "[PoC] ESP32 I2S + HVCC (external generator) build/flash"

# Preflight: ensure required tools are available
command -v hvcc >/dev/null 2>&1 || { echo "Error: hvcc not found on PATH."; echo "Install Heavy (HVCC) and ensure 'hvcc' is available."; exit 1; }

GEN_DIR="generated/espidf_app"
echo "Generating ESP-IDF app via HVCC external generator -> ${GEN_DIR}"
rm -rf "${GEN_DIR}"
mkdir -p "$(dirname "${GEN_DIR}")"
PYTHONPATH="${PYTHONPATH:-}":"$(pwd)" hvcc main/test.pd -G c2espidf -o "${GEN_DIR}"
HVCC_STATUS=$?
if [ ${HVCC_STATUS} -ne 0 ]; then
    echo "HVCC generation failed with status ${HVCC_STATUS}. Ensure the external generator module is discoverable."
    echo "Tip: this script sets PYTHONPATH to current directory for hvcc to import local modules."
    exit ${HVCC_STATUS}
fi

# Ensure ESP-IDF scaffolding exists only if generator didn't create it
echo "Ensuring ESP-IDF project layout in ${GEN_DIR}"
mkdir -p "${GEN_DIR}/main/hvcc/c"
if [ -d "${GEN_DIR}/c" ]; then
    cp -a "${GEN_DIR}/c/." "${GEN_DIR}/main/hvcc/c/"
fi

if [ ! -f "${GEN_DIR}/CMakeLists.txt" ]; then
    cat > "${GEN_DIR}/CMakeLists.txt" <<'EOF'
cmake_minimum_required(VERSION 3.16)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(hello_world)
EOF
fi

if [ ! -f "${GEN_DIR}/main/CMakeLists.txt" ]; then
    cat > "${GEN_DIR}/main/CMakeLists.txt" <<'EOF'
cmake_minimum_required(VERSION 3.16)

set(HVCC_OUT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/hvcc")

file(GLOB HVCC_SOURCES
        "${HVCC_OUT_DIR}/c/*.c"
        "${HVCC_OUT_DIR}/c/*.cpp"
)

if(NOT HVCC_SOURCES)
        message(WARNING "HVCC: No sources found in ${HVCC_OUT_DIR}/c. Build will proceed with app sources only.")
endif()

idf_component_register(
        SRCS
                "poc_esp32_hvcc_i2s.c"
                ${HVCC_SOURCES}
        INCLUDE_DIRS 
                "."
                "hvcc/c"
        REQUIRES driver
)
EOF
fi

if [ ! -f "${GEN_DIR}/main/poc_esp32_hvcc_i2s.c" ]; then
    cp -a main/poc_esp32_hvcc_i2s.c "${GEN_DIR}/main/poc_esp32_hvcc_i2s.c"
fi

# Patch heavy format specifiers and headers in copied sources
HV_MSG_PATH="${GEN_DIR}/main/hvcc/c/HvMessage.c"
HV_UTILS_PATH="${GEN_DIR}/main/hvcc/c/HvUtils.h"
if [ -f "${HV_MSG_PATH}" ]; then
    sed -i 's/\"0x%X\"/\"0x%\" PRIX32/g' "${HV_MSG_PATH}"
fi
if [ -f "${HV_UTILS_PATH}" ]; then
    grep -q "inttypes.h" "${HV_UTILS_PATH}" || sed -i 's/#include <stdint.h>/#include <stdint.h>\n#include <inttypes.h>/' "${HV_UTILS_PATH}"
fi

if command -v idf.py >/dev/null 2>&1; then
    echo "Setting target to esp32"
    idf.py set-target esp32
    echo "Building firmware in ${GEN_DIR}"
    cd "${GEN_DIR}"
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
else
    echo "idf.py not found. Project generated at ${GEN_DIR}."
    echo "To build:"
    echo "  . \"$HOME/esp/esp-idf/export.sh\""
    echo "  cd ${GEN_DIR} && idf.py set-target esp32 && idf.py build && idf.py -p /dev/ttyUSB0 flash"
fi
