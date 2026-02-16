# Proof of Concept: ESP32 I2S + HVCC (Pure Data Output)

This repository is a small proof of concept showing how to:
- Compile a Pure Data patch with HVCC (Heavy) and run it on ESP32.
- Stream the generated audio to an external DAC over I2S using ESP-IDF.

Focus is output-only: the PD patch produces audio; there is no input or control surface here. Default configuration is stereo at 48 kHz.

Flow overview:

```
Pure Data (test.pd)
        │
    HVCC (Heavy)
        │  generates C/C++
        ▼
    ESP-IDF build
        │
    I2S driver → DAC → audio out
```

## Prerequisites
- ESP-IDF installed and environment exported (so `idf.py` is available).
- HVCC (Heavy) installed and `hvcc` available on your `PATH`.
- An ESP32 board connected via serial (e.g., `/dev/ttyUSB0`).

## Quick Start
1) Open an ESP-IDF shell (Linux/macOS):
    ```bash
    . "$HOME/esp/esp-idf/export.sh"
    ```
2) Generate HVCC code, build, and flash via the script:
    ```bash
    chmod +x export.sh
    ./export.sh
    ```
    What happens:
    - Preflight checks for `hvcc` and `idf.py`, sets target `esp32`
    - Regenerates HVCC sources from [main/test.pd](main/test.pd) into [main/hvcc](main/hvcc)
    - Builds the firmware with ESP-IDF
    - Flashes the device (auto or via `ESPPORT`/`PORT` env)

If needed, set the serial port explicitly afterward:
```bash
export ESPPORT=/dev/ttyUSB0
./export.sh
```

## Pin Mapping (ESP32 → DAC)
- WS (LRCK): GPIO26
- BCLK: GPIO27
- DOUT: GPIO25
- MCLK: not used

Update the pins in [main/poc_esp32_hvcc_i2s.c](main/poc_esp32_hvcc_i2s.c) if your wiring differs.

## Files of Interest
- [main/test.pd](main/test.pd): Pure Data patch compiled by HVCC.
- [main/poc_esp32_hvcc_i2s.c](main/poc_esp32_hvcc_i2s.c): Encapsulated, commented example for I2S + Heavy.
- [main/CMakeLists.txt](main/CMakeLists.txt): Regenerates HVCC sources at configure time if `hvcc` is available.
- [export.sh](export.sh): One-liner workflow to regenerate, build, and flash.

## Notes & Limitations
- Output-only PoC: ensure your PD patch sends audio to outlets (e.g., `dac~`).
- Default sample rate: 48 kHz. Change in [main/poc_esp32_hvcc_i2s.c](main/poc_esp32_hvcc_i2s.c).
- Uses ESP-IDF standard I2S driver (`i2s_std`). Tested with stereo.

## Troubleshooting
- `hvcc not found`: Install Heavy (HVCC) and add it to your `PATH`.
- `idf.py not found`: Export ESP-IDF (`. "$HOME/esp/esp-idf/export.sh"`).
- Build error with `uint32_t` format specifiers: handled automatically by [main/CMakeLists.txt](main/CMakeLists.txt), which patches generated files to use `PRIX32` and adds `<inttypes.h>`.
- Serial port issues: set `ESPPORT` or `PORT` env to your device (e.g., `/dev/ttyUSB0`).

## Keep the Repo Clean
This project includes a `.gitignore` that excludes build outputs and HVCC-generated sources, keeping only the pedagogical essentials. Regenerate as needed with [export.sh](export.sh).

## Credits
- HVCC / Heavy by Enzien Audio.
- ESP-IDF by Espressif Systems.

