#include "switch.h"
#include "esp_timer.h"

Switch::Switch() {}
Switch::~Switch() {}

void Switch::Init(gpio_num_t pin, Type t, Polarity pol)
{
    pin_ = pin;
    t_ = t;
    flip_ = (pol == POLARITY_INVERTED);
    state_ = 0;
    updated_ = false;
    last_update_ = esp_timer_get_time() / 1000;

    gpio_config_t io_conf = {
        .pin_bit_mask = 1ULL << pin_,
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = (pol == POLARITY_INVERTED) ? GPIO_PULLUP_ENABLE : GPIO_PULLUP_DISABLE,
        .pull_down_en = (pol == POLARITY_INVERTED) ? GPIO_PULLDOWN_DISABLE : GPIO_PULLDOWN_ENABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    gpio_config(&io_conf);
}

void Switch::Debounce()
{
    uint32_t now = esp_timer_get_time() / 1000;
    updated_ = false;

    if (now - last_update_ >= 1)
    {
        last_update_ = now;
        updated_ = true;

        bool raw = gpio_get_level(pin_);
        bool val = flip_ ? !raw : raw;
        state_ = (state_ << 1) | val;

        if (state_ == 0x7f)
            rising_edge_time_ = now;
    }
}

bool Switch::RisingEdge() const { return updated_ ? state_ == 0x7f : false; }
bool Switch::FallingEdge() const { return updated_ ? state_ == 0x80 : false; }
bool Switch::Pressed() const { return state_ == 0xff; }

bool Switch::RawState() const
{
    bool raw = gpio_get_level(pin_);
    return flip_ ? !raw : raw;
}

float Switch::TimeHeldMs() const
{
    uint32_t now = esp_timer_get_time() / 1000;
    return Pressed() ? now - rising_edge_time_ : 0;
}
