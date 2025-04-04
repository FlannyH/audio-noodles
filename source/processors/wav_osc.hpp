#pragma once

#include "../processor.hpp"
#include "../adsr.hpp"
#include <vector>

struct Voice {
    VolEnv vol_env;
    uint8_t key;
    uint8_t velocity;
};

struct WavOsc : Processor {
    WavOsc();
    void process_block(const size_t n_frames, float* output) override;
    virtual void key_on(uint8_t key, uint8_t velocity) override;
    virtual void key_off(uint8_t key) override;

    std::vector<Voice> voice_pool;
    VolEnvParams params;
};
