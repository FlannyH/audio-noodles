#include "wav_osc.hpp"
#include "../mixer.hpp"

#include <cmath>

WavOsc::WavOsc() {
    this->voice_pool.resize(16);
}

void WavOsc::process_block(const size_t n_frames, float *output) {
    const double sample_length_sec = 1.0 / Mixer::sample_rate();
    double time = Mixer::block_start_time();

    for (size_t i = 0; i < n_frames; ++i) {
        output[2*i + 0] = 0.0f;
        output[2*i + 1] = 0.0f;
        for (size_t j = 0; j < this->voice_pool.size(); ++j) {
            if (this->voice_pool[j].is_playing == 0) 
                continue;

            double key_relative_to_a4 = ((double)this->voice_pool[j].key) - 69.0;
            double frequency = 440.0 * pow(2.0, key_relative_to_a4 / 12.0); // todo: non-440 hz tuning, pitch wheel, mod vibrato, microtonality
            double sample_l = sin(time * frequency * 2.0 * 3.14159265);
            double sample_r = sin(time * frequency * 2.0 * 3.14159265);
            double volume_multiplier = ((double)this->voice_pool[j].velocity) / 127.0;
            output[2*i + 0] += (float)(sample_l * volume_multiplier * Mixer::global_volume());
            output[2*i + 1] += (float)(sample_r * volume_multiplier * Mixer::global_volume());
        }
        time += sample_length_sec;
    }
}

void WavOsc::key_on(uint8_t key, uint8_t velocity) {
    for (size_t i = 0; i < this->voice_pool.size(); ++i) {
        if (this->voice_pool[i].is_playing == 1) 
            continue;

        this->voice_pool[i].key = key;
        this->voice_pool[i].velocity = velocity;
        this->voice_pool[i].is_playing = 1;
        break;
    }
}

void WavOsc::key_off(uint8_t key) {
    // todo: adsr
    for (size_t i = 0; i < this->voice_pool.size(); ++i) {
        if (this->voice_pool[i].key == key) {
            this->voice_pool[i].is_playing = 0;
        }
    }
}
