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
#include "esp_adc/adc_oneshot.h"
// Heavy (hvcc) generated patch interface
#include "hvcc/c/Heavy_heavy.h"
#include "hvcc/c/HvHeavy.h"

//  configure I2S TX for 48kHz stereo on specific pins.
static i2s_chan_handle_t init_i2s_tx(uint32_t sample_rate, gpio_num_t ws, gpio_num_t bclk, gpio_num_t dout) {
    i2s_chan_handle_t tx_handle = NULL;
    i2s_chan_config_t chan_cfg = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_AUTO, I2S_ROLE_MASTER);
    chan_cfg.dma_desc_num = 4;
    chan_cfg.dma_frame_num = 256;
    ESP_ERROR_CHECK(i2s_new_channel(&chan_cfg, &tx_handle, NULL));

    i2s_std_config_t std_cfg = {
        .clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(sample_rate),
        .slot_cfg = I2S_STD_PHILIPS_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_STEREO),
        .gpio_cfg = {
            .mclk = I2S_GPIO_UNUSED,
            .bclk = bclk,
            .ws   = ws,
            .dout = dout,
            .din  = I2S_GPIO_UNUSED,
            .invert_flags = { .mclk_inv = false, .bclk_inv = false, .ws_inv = false },
        },
    };
    std_cfg.slot_cfg.slot_mask = I2S_STD_SLOT_BOTH;
    ESP_ERROR_CHECK(i2s_channel_init_std_mode(tx_handle, &std_cfg));
    ESP_ERROR_CHECK(i2s_channel_enable(tx_handle));
    return tx_handle;
}

//  create a Heavy (HVCC) audio context for the given sample rate.
static HeavyContextInterface* init_heavy(uint32_t sample_rate, int *out_channels) {
    HeavyContextInterface *hv_ctx = hv_heavy_new((double) sample_rate);
    int ch = hv_getNumOutputChannels(hv_ctx);
    if (ch <= 0) ch = 1; // fallback to mono if patch declares none
    *out_channels = ch;
    return hv_ctx;
}

//  process audio in blocks and send to I2S.
static void run_audio_loop(i2s_chan_handle_t tx, HeavyContextInterface *hv_ctx, int num_out_channels) {
    const int frames_per_block = 256; // HVCC likes multiples of 8
    float hv_out[frames_per_block * 2];
    int16_t samples[frames_per_block * 2];
    while (1) {
        int s = hv_processInline(hv_ctx, NULL, hv_out, frames_per_block);
        if (s <= 0) { vTaskDelay(1); continue; }
        for (int i = 0; i < s; ++i) {
            float l = hv_out[i];
            float r = (num_out_channels >= 2) ? hv_out[i + s] : l;
            if (l > 1.0f) l = 1.0f; else if (l < -1.0f) l = -1.0f;
            if (r > 1.0f) r = 1.0f; else if (r < -1.0f) r = -1.0f;
            samples[2 * i]     = (int16_t)(l * 32767.0f);
            samples[2 * i + 1] = (int16_t)(r * 32767.0f);
        }
        size_t written = 0;
        if (i2s_channel_write(tx, samples, (size_t)(s * 2 * sizeof(int16_t)), &written, portMAX_DELAY) != ESP_OK) {
            vTaskDelay(1);
        }
    }
}

typedef struct {
    gpio_num_t pin;
    const char *recv;
    int invert;
    hv_uint32_t hash;
    int last_level;
} ButtonMap;

typedef struct {
    adc_channel_t ch;
    const char *recv;
    hv_uint32_t hash;
} AdcMap;

typedef struct {
    HeavyContextInterface *hv;
    adc_oneshot_unit_handle_t adc;
    AdcMap *adc_map;
    int adc_count;
    ButtonMap *btn_map;
    int btn_count;
} ControlCtx;

static void controls_task(void *arg) {
    ControlCtx *ctx = (ControlCtx *) arg;
    const TickType_t delay = pdMS_TO_TICKS(10);
    while (1) {
        for (int i = 0; i < ctx->btn_count; ++i) {
            int lvl = gpio_get_level(ctx->btn_map[i].pin);
            if (ctx->btn_map[i].invert) lvl = !lvl;
            if (lvl != ctx->btn_map[i].last_level) {
                ctx->btn_map[i].last_level = lvl;
                hv_sendFloatToReceiver(ctx->hv, ctx->btn_map[i].hash, (float) lvl);
            }
        }
        for (int i = 0; i < ctx->adc_count; ++i) {
            int raw = 0;
            if (adc_oneshot_read(ctx->adc, ctx->adc_map[i].ch, &raw) == ESP_OK) {
                float v = (float) raw / 4095.0f;
                hv_sendFloatToReceiver(ctx->hv, ctx->adc_map[i].hash, v);
            }
        }
        vTaskDelay(delay);
    }
}

void app_main(void)
{
    // Pin mapping (ESP32 -> DAC). Adjust for your board.
    const gpio_num_t I2S_WS   = GPIO_NUM_26;  // LRCK/WS
    const gpio_num_t I2S_BCLK = GPIO_NUM_27;  // BCLK
    const gpio_num_t I2S_DOUT = GPIO_NUM_25;  // DATA OUT
    const uint32_t sample_rate = 48000;       // 48 kHz

    i2s_chan_handle_t tx = init_i2s_tx(sample_rate, I2S_WS, I2S_BCLK, I2S_DOUT);
    int num_out_channels = 0;
    HeavyContextInterface *hv_ctx = init_heavy(sample_rate, &num_out_channels);

    // Map hardware controls to PD receivers (like pd2dsy-style mapping).
    // Buttons: GPIO32 as input with pull-up, send 0/1 float to PD receiver.
    static ButtonMap buttons[] = {
        { GPIO_NUM_32, "button1", 1, 0, -1 },
    };
    for (int i = 0; i < (int)(sizeof(buttons)/sizeof(buttons[0])); ++i) {
        buttons[i].hash = hv_stringToHash(buttons[i].recv);
        gpio_config_t io = {
            .pin_bit_mask = (1ULL << buttons[i].pin),
            .mode = GPIO_MODE_INPUT,
            .pull_up_en = 1,
            .pull_down_en = 0,
            .intr_type = GPIO_INTR_DISABLE,
        };
        gpio_config(&io);
        buttons[i].last_level = -1;
    }

    // ADC (knobs): ADC1_CH5 (GPIO33) sends 0..1 float to PD receiver.
    static AdcMap knobs[] = {
        { ADC_CHANNEL_5, "knob1", 0 },
    };
    for (int i = 0; i < (int)(sizeof(knobs)/sizeof(knobs[0])); ++i) {
        knobs[i].hash = hv_stringToHash(knobs[i].recv);
    }

    adc_oneshot_unit_handle_t adc_unit;
    adc_oneshot_unit_init_cfg_t unit_cfg = {
        .unit_id = ADC_UNIT_1,
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&unit_cfg, &adc_unit));
    for (int i = 0; i < (int)(sizeof(knobs)/sizeof(knobs[0])); ++i) {
        adc_oneshot_chan_cfg_t chan_cfg = {
            .bitwidth = ADC_BITWIDTH_DEFAULT,
            .atten = ADC_ATTEN_DB_11,
        };
        ESP_ERROR_CHECK(adc_oneshot_config_channel(adc_unit, knobs[i].ch, &chan_cfg));
    }

    ControlCtx cctx = {
        .hv = hv_ctx,
        .adc = adc_unit,
        .adc_map = knobs,
        .adc_count = (int)(sizeof(knobs)/sizeof(knobs[0])),
        .btn_map = buttons,
        .btn_count = (int)(sizeof(buttons)/sizeof(buttons[0])),
    };
    xTaskCreate(controls_task, "controls", 4096, &cctx, 5, NULL);

    run_audio_loop(tx, hv_ctx, num_out_channels);
}
