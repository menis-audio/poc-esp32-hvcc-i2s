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
Recommended: use the HVCC external generator to scaffold a standalone ESP-IDF project (no Python execution inside HVCC; `idf.py` is only used by you, externally).

1) Open an ESP-IDF shell (Linux/macOS):
    ```bash
    . "$HOME/esp/esp-idf/export.sh"
    ```
2) Generate an ESP-IDF app with the custom generator:
    ```bash
    hvcc main/test.pd -G c2espidf -o generated/espidf_app
    ```
    This creates a fresh ESP-IDF project in `generated/espidf_app` containing:
    - HVCC C sources copied under `main/hvcc/c`
    - An I2S + Heavy wrapper in [generated/espidf_app/main/poc_esp32_hvcc_i2s.c](generated/espidf_app/main/poc_esp32_hvcc_i2s.c)
    - Minimal CMake files to build with ESP-IDF

3) Build and flash the generated project (outside HVCC):
    ```bash
    cd generated/espidf_app
    idf.py set-target esp32
    idf.py build
    idf.py -p /dev/ttyUSB0 flash
    ```

Note: Building requires an ESP-IDF environment. The external HVCC generator does not execute Python; you run `idf.py` (or another ESP32 build system) yourself, outside HVCC.

One-liner:
- Use [export.sh](export.sh) to run the generator and build/flash automatically.
    ```bash
    chmod +x export.sh
    ./export.sh
    ```

If needed, set the serial port explicitly:
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
- [main/CMakeLists.txt](main/CMakeLists.txt): Consumes HVCC sources if present under `main/hvcc`; does not regenerate.
- [export.sh](export.sh): One-liner workflow to generate a standalone ESP-IDF app and build/flash it.

## External Generator
- Custom HVCC generator module: [c2espidf.py](c2espidf.py)
- Invoke with: `hvcc main/test.pd -G c2espidf -o generated/espidf_app`
- Behavior:
    - Copies HVCC C sources from HVCC compile stage into `main/hvcc/c`
    - Writes minimal ESP-IDF `CMakeLists.txt` and wrapper [poc_esp32_hvcc_i2s.c](generated/espidf_app/main/poc_esp32_hvcc_i2s.c)
    - Applies safe printf fixes in `HvMessage.c` and adds `<inttypes.h>` to `HvUtils.h`
- Deliberately does not call `idf.py`: building and flashing happen outside HVCC, so the generator works within pyinstaller-packed environments.

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

## Community & Context
This proof of concept was created following a Discord discussion with collaborators who suggested iterating via HVCC external generators for easier development. The implementation draws inspiration from an ESP8266 example shared by "dreamer" and aims to serve as a base to explore more generators and targets.

- Goal: a simple starting point to study HVCC generators and iterate quickly.
- Feedback: please open issues and suggestions on the public repo.
- Repo: https://github.com/menis-audio/poc-esp32-hvcc-i2s
- References:
    - HVCC external generator docs: https://wasted-audio.github.io/hvcc/docs/03.gen.external.html
    - ESP32 HVCC reference: https://github.com/sinneb/esp32-hvcc
    - ESP8266 Heavy example: https://github.com/Simon-L/heavy_esp8266
    - Pure Data on Arduino Audio Tools: https://github.com/pschatzmann/arduino-audio-tools/wiki/Pure-Data

Thanks to everyone for the helpful pointers and shared code. Any questions or critiques are welcome—I'll respond as soon as possible.

