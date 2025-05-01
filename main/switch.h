#pragma once
#ifndef SWITCH_ESP32_H
#define SWITCH_ESP32_H

#include "driver/gpio.h"

class Switch
{
  public:
    enum Type
    {
        TYPE_TOGGLE,
        TYPE_MOMENTARY,
    };

    enum Polarity
    {
        POLARITY_NORMAL,
        POLARITY_INVERTED,
    };

    Switch();
    ~Switch();

    void Init(gpio_num_t pin, Type t = TYPE_MOMENTARY, Polarity pol = POLARITY_INVERTED);
    void Debounce();

    bool RisingEdge() const;
    bool FallingEdge() const;
    bool Pressed() const;
    bool RawState() const;
    float TimeHeldMs() const;

  private:
    gpio_num_t pin_;
    Type t_;
    bool flip_;
    uint8_t state_;
    bool updated_;
    uint32_t last_update_;
    uint32_t rising_edge_time_;
};

#endif // SWITCH_ESP32_H
