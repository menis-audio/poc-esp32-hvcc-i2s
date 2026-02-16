/*
 * SPDX-FileCopyrightText: 2010-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include <stdio.h>
#include <inttypes.h>
#include <math.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
//#include "esp_chip_info.h"
//#include "esp_flash.h"
//#include "esp_system.h"
// I2S (STD) TX for audio @ 48kHz
#include "driver/i2s_std.h"
#include "driver/gpio.h"
// Heavy (hvcc) generated patch interface
#include "hvcc/c/Heavy_heavy.h"
#include "hvcc/c/HvHeavy.h"

void app_main(void)
{
    // Pin mapping per your wiring (ESP32 -> DAC)
    // WS  = GPIO26, BCLK = GPIO27, DOUT = GPIO25, MCLK unused
    const gpio_num_t I2S_WS   = GPIO_NUM_26;
    const gpio_num_t I2S_BCLK = GPIO_NUM_27;
    const gpio_num_t I2S_DOUT = GPIO_NUM_25;

    const uint32_t sample_rate = 48000;   // 48 kHz

    i2s_chan_handle_t tx_handle = NULL;

    i2s_chan_config_t chan_cfg = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_AUTO, I2S_ROLE_MASTER);
    // Optional: tune DMA sizing for smooth streaming
    chan_cfg.dma_desc_num = 4;   // number of DMA descriptors
    chan_cfg.dma_frame_num = 256; // frames per DMA buffer (frame = L+R samples)

    ESP_ERROR_CHECK(i2s_new_channel(&chan_cfg, &tx_handle, NULL));

    i2s_std_config_t std_cfg = {
        .clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(sample_rate),
        .slot_cfg = I2S_STD_PHILIPS_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_STEREO),
        .gpio_cfg = {
            .mclk = I2S_GPIO_UNUSED,
            .bclk = I2S_BCLK,
            .ws   = I2S_WS,
            .dout = I2S_DOUT,
            .din  = I2S_GPIO_UNUSED,
            .invert_flags = {
                .mclk_inv = false,
                .bclk_inv = false,
                .ws_inv   = false,
            },
        },
    };

    // Ensure both channels (L/R) are active in stereo
    std_cfg.slot_cfg.slot_mask = I2S_STD_SLOT_BOTH;

    ESP_ERROR_CHECK(i2s_channel_init_std_mode(tx_handle, &std_cfg));

    // Optional: preload to avoid initial silence
    // Not strictly needed here, we enable and immediately start sending.
    ESP_ERROR_CHECK(i2s_channel_enable(tx_handle));

    // Heavy context init
    HeavyContextInterface *hv_ctx = hv_heavy_new((double) sample_rate);
    int num_out_channels = hv_getNumOutputChannels(hv_ctx);
    if (num_out_channels <= 0) {
        // Fallback to mono if patch has no audio outs
        num_out_channels = 1;
    }
    const int frames_per_block = 256; // must be a multiple of 8 for hv
    float hv_out[frames_per_block * 2]; // support up to 2 channels (LLLLRRRR)
    int16_t samples[frames_per_block * 2]; // interleaved L/R

    while (1) {
        int s = hv_processInline(hv_ctx, NULL, hv_out, frames_per_block);
        if (s <= 0) {
            // If nothing processed, yield briefly
            vTaskDelay(1);
            continue;
        }

        for (int i = 0; i < s; ++i) {
            float l = hv_out[i];
            float r = (num_out_channels >= 2) ? hv_out[i + s] : l;
            if (l > 1.0f) l = 1.0f; else if (l < -1.0f) l = -1.0f;
            if (r > 1.0f) r = 1.0f; else if (r < -1.0f) r = -1.0f;
            int16_t li = (int16_t)(l * 32767.0f);
            int16_t ri = (int16_t)(r * 32767.0f);
            samples[2 * i]     = li; // Left
            samples[2 * i + 1] = ri; // Right
        }

        size_t written = 0;
        esp_err_t err = i2s_channel_write(tx_handle, samples, (size_t)(s * 2 * sizeof(int16_t)), &written, portMAX_DELAY);
        if (err != ESP_OK) {
            vTaskDelay(1);
        }
    }
}
