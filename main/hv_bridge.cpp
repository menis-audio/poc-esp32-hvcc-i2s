#include <stdint.h>
#include "hv_config.h"
// Include generated hvcc header: Heavy_<name>.h
// The file should exist under main/hvcc/c after running hvcc with -g c.
#include HV_HEADER_PATH

static HeavyContextInterface* hvCtx = nullptr;
static int hvNumOut = 0;

void hv_init_bridge(uint32_t sample_rate) {
    if (hvCtx) return;
    hvCtx = (HeavyContextInterface*) (hv_mySynth_new((double) sample_rate));
    hvNumOut = hv_getNumOutputChannels(hvCtx);
}

// Process a single sample; writes stereo floats to outL/outR.
// If patch outputs mono, duplicate to both channels.
void hv_process_one(float& outL, float& outR) {
    if (!hvCtx) { outL = 0.0f; outR = 0.0f; return; }
    const int blockSize = 1; // Xtensa (ESP32) uses HV_SIMD_NONE; block of 1 is valid.

    if (hvNumOut <= 0) { outL = 0.0f; outR = 0.0f; return; }

    float outBuf[2] = {0.0f, 0.0f};

    if (hvNumOut == 1) {
        hv_processInline(hvCtx, nullptr, outBuf, blockSize);
        outL = outBuf[0];
        outR = outBuf[0];
    } else {
        // Uninterleaved: [LLLLRRRR] with blockSize=1
        hv_processInline(hvCtx, nullptr, outBuf, blockSize);
        outL = outBuf[0];
        outR = outBuf[1];
    }
}
