#pragma once

#include <stdint.h>
#include <stddef.h>
#include "driver/adc.h"

namespace daisy
{

constexpr int DSY_ADC_MAX_CHANNELS = 8;
constexpr float DSY_ADC_MAX_RESOLUTION = 4095.0f;

// Map logical analog pin names to ADC1 channels (only ADC1 is safe if using Wi-Fi)
enum AnalogPin
{
    A0 = ADC1_CHANNEL_0, // GPIO36
    A1 = ADC1_CHANNEL_1, // GPIO37
    A2 = ADC1_CHANNEL_2, // GPIO38
    A3 = ADC1_CHANNEL_3, // GPIO39
    A4 = ADC1_CHANNEL_4, // GPIO32
    A5 = ADC1_CHANNEL_5, // GPIO33
    A6 = ADC1_CHANNEL_6, // GPIO34
    A7 = ADC1_CHANNEL_7  // GPIO35
};

struct AdcChannelConfig
{
    /** Initialize the channel using a mapped AnalogPin */
    void InitSingle(AnalogPin pin);

    /** Read raw 12-bit ADC value */
    uint16_t ReadRaw();

    /** Read voltage in the 0.0 to 3.3V range (approximate) */
    float ReadVoltage();

  private:
    adc1_channel_t channel_;
};

class AdcHandle
{
  public:
    /** Initialize multiple ADC channels with optional oversampling count */
    void Init(AdcChannelConfig* cfg, size_t num_channels, int oversample_count = 1);

    void Start() {}
    void Stop() {}

    /** Get raw 12-bit value from a configured channel */
    uint16_t Get(uint8_t chn);

    /** Get voltage (0.0 - 3.3V) from a configured channel */
    float GetFloat(uint8_t chn);

    /** Set smoothing factor and update rate */
    void ConfigureSmoothing(float alpha, int update_interval);

    /** Call this periodically from main loop */
    void Update();

    /** Get smoothed voltage */
    float GetSmoothed(uint8_t chn);

  private:
    AdcChannelConfig* channels_ = nullptr;
    size_t num_channels_ = 0;
    int oversample_count_ = 1;
    float smoothing_alpha_ = 0.1f;
    int update_counter_ = 0;
    int update_interval_ = 100;
    float smoothed_values_[DSY_ADC_MAX_CHANNELS] = {0};
};

} // namespace daisy
