# Proof of Concept: ESP32 I2S + HVCC (Pure Data Output)

This repository is a small proof of concept showing how to:
- Compile a Pure Data patch with HVCC (Heavy) and run it on ESP32.
- Stream the generated audio to an external DAC over I2S using ESP-IDF.

Focus is output-only: the PD patch produces audio; there is no input or control surface here. Default configuration is stereo at 48 kHz.

## Prerequisites
- ESP-IDF installed and environment exported (so `idf.py` is available).
- HVCC (Heavy) installed and `hvcc` available on your `PATH`.
- An ESP32 board connected via serial (e.g., `/dev/ttyUSB0`).

## Quick Start
1) Open an ESP-IDF shell (Linux/macOS):
    ```bash
    . "$HOME/esp/esp-idf/export.sh"
    ```
2) Run the project script:
    ```bash
    chmod +x export.sh
    ./export.sh
    ```
    The script will:
    - Remove any previous `main/hvcc` output
    - Compile `main/test.pd` with HVCC into `main/hvcc`
    - Build the firmware and flash it to the connected ESP32

If needed, set the serial port explicitly afterward:
```bash
idf.py -p <PORT> flash
```

## Pin Mapping (ESP32 → DAC)
- WS (LRCK): GPIO26
- BCLK: GPIO27
- DOUT: GPIO25
- MCLK: not used

Update the pins in [main/poc_esp32_hvcc_i2s.c](main/poc_esp32_hvcc_i2s.c) if your wiring differs.

## Files of Interest
- [main/test.pd](main/test.pd): Pure Data patch compiled by HVCC.
- [main/poc_esp32_hvcc_i2s.c](main/poc_esp32_hvcc_i2s.c): I2S setup and streaming from the Heavy context.
- [export.sh](export.sh): Convenience script to regenerate HVCC output, build, and flash.

## Notes & Limitations
- Output-only PoC: ensure your PD patch generates audio to standard outlets.
- Sample rate is set to 48 kHz in the C file; adjust if required.
- Uses the ESP-IDF standard I2S driver (`i2s_std`).

