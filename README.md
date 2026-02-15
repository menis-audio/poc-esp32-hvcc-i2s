# ESP32 I2S 48 kHz Tone (220 Hz)

- Outputs a continuous 220 Hz sine wave over I2S at 48 kHz.
- Uses the new ESP-IDF I2S standard driver (`esp_driver_i2s`).

## Pin Mapping (ESP32 → DAC)
- WS (LRCK): GPIO26
- BCLK: GPIO27
- DOUT: GPIO25
- MCLK: not used

Update the pins in [main/poc_esp32_hvcc_i2s.c](main/poc_esp32_hvcc_i2s.c) if your wiring differs.

## Build and Flash
1) Open an ESP-IDF environment:
     - Linux/macOS:
         - `. $HOME/esp/esp-idf/export.sh`
2) Build, flash, and monitor:
```
idf.py set-target esp32
idf.py build
idf.py -p <PORT> flash monitor
```
Replace `<PORT>` with your serial port (e.g. `/dev/ttyUSB0`).

## Project Layout
```
├── CMakeLists.txt
├── main
│   ├── CMakeLists.txt
│   └── poc_esp32_hvcc_i2s.c   // I2S init + 220 Hz generator
└── README.md
```
