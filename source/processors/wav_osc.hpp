#pragma once

#include "../processor.hpp"

struct WavOsc : Processor {
    void process_block(const size_t n_frames, float* output) override;
};
