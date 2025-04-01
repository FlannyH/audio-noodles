#include "processor.hpp"
#include "mixer.hpp"

#include <cmath>

void Processor::process_block(const size_t n_frames, float* output) {
    const double sample_length_sec = 1.0 / Mixer::sample_rate();
    double time = Mixer::block_start_time();

    for (size_t i = 0; i < n_frames; ++i) {
        double sample_l = sin(time * 440.0 * 2.0 * 3.14159265);
        double sample_r = sin(time * 440.0 * 2.0 * 3.14159265);
        output[2*i + 0] = (float)(sample_l * Mixer::global_volume());
        output[2*i + 1] = (float)(sample_r * Mixer::global_volume());
        time += sample_length_sec;
    } 
}
