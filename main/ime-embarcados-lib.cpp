#include "driver/i2s_std.h"
#include "freertos/FreeRTOS.h"
#include "config.h"

#ifdef HV_ENABLED
#include "hv_bridge.h"
#endif

static uint32_t sr = 48000;

void audio_callback()
{
#ifdef HV_ENABLED
    float l, r;
    hv_process_one(l, r);
    to_audio_write(l, r);
#else
    // Fallback: silence when HVCC patch is not present
    to_audio_write(0.0f, 0.0f);
#endif
}

extern "C" void app_main(void)
{
    audio_init(sr);
#ifdef HV_ENABLED
    hv_init_bridge(sr);
#endif
    while (1)
    {
        audio_callback();
    }
}

