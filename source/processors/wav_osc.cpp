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
        for (auto& voice : this->voice_pool) {
            if (voice.vol_env.stage == VolEnvStage::idle) 
                continue;


            voice.vol_env.tick(sample_length_sec, this->params);
            double key_relative_to_a4 = ((double)voice.key) - 69.0; // nice
            double frequency = 440.0 * pow(2.0, key_relative_to_a4 / 12.0); // todo: non-440 hz tuning, pitch wheel, mod vibrato, microtonality
            double sample_l = sin(time * frequency * 2.0 * 3.14159265);
            double sample_r = sin(time * frequency * 2.0 * 3.14159265);
            double volume_multiplier = ((double)voice.velocity) / 127.0;
            double adsr_volume = voice.vol_env.adsr_volume;
            double final_volume = volume_multiplier * adsr_volume * Mixer::global_volume();
            final_volume = final_volume * final_volume;
            output[2*i + 0] += (float)(sample_l * final_volume);
            output[2*i + 1] += (float)(sample_r * final_volume);
        }
        time += sample_length_sec;
    }
}

void WavOsc::key_on(uint8_t key, uint8_t velocity) {
    for (auto& voice : this->voice_pool) {
        if (voice.vol_env.stage != VolEnvStage::idle) 
            continue;

        voice.key = key;
        voice.velocity = velocity;
        voice.vol_env.stage = VolEnvStage::delay;
        break;
    }
}

void WavOsc::key_off(uint8_t key) {
    for (auto& voice : this->voice_pool) {
        if (voice.key == key) {
            voice.vol_env.stage = VolEnvStage::release;
        }
    }
}
