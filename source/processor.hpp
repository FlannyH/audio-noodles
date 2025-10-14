#pragma once
#include <cstdint>
#include <cstddef>

struct Processor {
    virtual void process_block(const size_t n_samples, float* output) = 0;
    virtual void key_on(uint8_t key, uint8_t velocity) {}
    virtual void key_off(uint8_t key) {}

    int32_t ui_panel_index = -1;
};
