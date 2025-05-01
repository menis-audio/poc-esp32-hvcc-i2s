#include "adc.h"
#include "driver/gpio.h"

using namespace daisy;

void AdcChannelConfig::InitSingle(AnalogPin pin)
{
    channel_ = static_cast<adc1_channel_t>(pin);
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(channel_, ADC_ATTEN_DB_11);
}

uint16_t AdcChannelConfig::ReadRaw()
{
    return adc1_get_raw(channel_);
}

float AdcChannelConfig::ReadVoltage()
{
    int raw = adc1_get_raw(channel_);
    return raw * (3.3f / DSY_ADC_MAX_RESOLUTION);
}

void AdcHandle::Init(AdcChannelConfig* cfg, size_t num_channels, int oversample_count)
{
    channels_ = cfg;
    num_channels_ = num_channels;
    oversample_count_ = oversample_count > 0 ? oversample_count : 1;

    for (size_t i = 0; i < num_channels_; i++)
    {
        smoothed_values_[i] = 0.0f;
    }
}

uint16_t AdcHandle::Get(uint8_t chn)
{
    if (chn >= num_channels_) return 0;
    return channels_[chn].ReadRaw();
}

float AdcHandle::GetFloat(uint8_t chn)
{
    if (chn >= num_channels_) return 0.0f;
    return channels_[chn].ReadVoltage();
}

void AdcHandle::ConfigureSmoothing(float alpha, int update_interval)
{
    smoothing_alpha_ = alpha;
    update_interval_ = update_interval;
}

void AdcHandle::Update()
{
    if (++update_counter_ >= update_interval_)
    {
        update_counter_ = 0;
        for (size_t i = 0; i < num_channels_; i++)
        {
            float sum = 0.0f;
            for (int j = 0; j < oversample_count_; j++)
            {
                sum += channels_[i].ReadVoltage();
            }
            float voltage = sum / oversample_count_;

            smoothed_values_[i] = smoothing_alpha_ * voltage +
                                  (1.0f - smoothing_alpha_) * smoothed_values_[i];
        }
    }
}

float AdcHandle::GetSmoothed(uint8_t chn)
{
    if (chn >= num_channels_) return 0.0f;
    return smoothed_values_[chn];
}
