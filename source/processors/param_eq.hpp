#pragma once

#include <memory>
#include "../processor.hpp"

struct Biquad {
    double b0, b1, b2;
    double a0, a1, a2;
    double x1, x2;
    double y1, y2;
    float tick(float x0);
};

struct Param {
    float f0;
    float q;
    float gain;
    float sample_rate;
    Biquad biquad[2];
};

Biquad identity();
Biquad low_pass(float f0, float q, float sample_rate);
Biquad low_shelf(float f0, float q, float gain, float sample_rate);
Biquad peak(float f0, float q, float gain, float sample_rate);
Biquad high_shelf(float f0, float q, float gain, float sample_rate);
Biquad high_pass(float f0, float q, float sample_rate);

struct ParamEq : Processor {
    ParamEq();
    virtual void audio_process_block(const size_t n_samples, const double output_sample_rate, const Sample* input, Sample* output, std::vector<std::shared_ptr<Processor>> targets) override;
    std::vector<Param> params;

private:
    void update_biquads();
};
