#pragma once
#include <stdint.h>

void hv_init_bridge(uint32_t sample_rate);
void hv_process_one(float& outL, float& outR);
