#pragma once
#include <cstdint>

struct Processor {
    virtual void process_block(const size_t n_samples, float* output) = 0;
    virtual void key_on(uint8_t key, uint8_t velocity) {}
    virtual void key_off(uint8_t key) {}
};
