#pragma once

struct Processor {
    virtual void process_block(const size_t n_samples, float* output) = 0;
};
