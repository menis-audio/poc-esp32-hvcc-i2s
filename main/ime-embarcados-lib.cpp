#include "driver/i2s_std.h"
#include "freertos/FreeRTOS.h"
#include "config.h"
#include "../DaisySP/Source/daisysp.h"
#include "adc.h"
#include "switch.h"
#include "esp_timer.h"

using namespace daisysp;
using namespace daisy;

Oscillator osc;
Adsr env;
Switch btn;

uint32_t sr = 48000;

AdcChannelConfig adc_cfg[1];
AdcHandle adc;
float adc_freq = 0.0f;

void audio_callback()
{
    // Process envelope with gate signal (true while pressed)
    float env_out = env.Process(btn.Pressed());
    float sig = osc.Process() * env_out;
    osc.SetFreq(adc_freq);
    to_audio_write(sig, sig);
}

extern "C" void app_main(void)
{
    // Init Oscillator
    osc.Init(sr);
    osc.SetWaveform(Oscillator::WAVE_SAW);
    osc.SetAmp(1.0f);

    // Init ADSR Envelope
    env.Init(sr);
    env.SetAttackTime(0.01f);   // fast attack
    env.SetDecayTime(0.1f);     // medium decay
    env.SetSustainLevel(0.7f);  // sustained level
    env.SetReleaseTime(0.2f);   // smooth release

    // Init Audio
    audio_init(sr);

    // Init ADC (GPIO34 = A6)
    adc_cfg[0].InitSingle(A6);
    adc.Init(adc_cfg, 1, 8);
    adc.ConfigureSmoothing(0.09f, 500); // alpha = 0.1, update every 500 calls

    // Init Switch (GPIO13)
    btn.Init(GPIO_NUM_13, Switch::TYPE_MOMENTARY, Switch::POLARITY_INVERTED);

    while (1)
    {
        // Update control inputs
        adc.Update();
        btn.Debounce();

        // Map ADC value to frequency
        adc_freq = 100.0f + adc.GetSmoothed(0) * 800.0f;

        // Optional: force retrigger on rising edge (smooth transition)
        if (btn.RisingEdge())
        {
            env.Retrigger(false); // soft retrigger, avoids clicks
        }

        // Process audio
        audio_callback();
    }
}
