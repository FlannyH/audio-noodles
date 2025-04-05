#include "wav_osc.hpp"
#include "../mixer.hpp"
#include "../common.hpp"

#include <cmath>

thread_local uint32_t noise_state = 0x796C694C;

double poly_blep(double t, double dt) {
    if (t < dt) {
        t /= dt;
        return t+t - t*t - 1.0;
    }
    else if (t > 1.0 - dt) {
        t = (t - 1.0) / dt;
        return t*t + t+t + 1.0;
    }
    return 0.0;
}

WavOsc::WavOsc() {
    this->voice_pool.resize(16);
}

void WavOsc::process_block(const size_t n_frames, float *output) {
    const double sample_length_sec = 1.0 / Mixer::sample_rate();
    double time = Mixer::block_start_time();

    for (size_t i = 0; i < n_frames; ++i) {
        for (auto& voice : this->voice_pool) {
            if (voice.vol_env.stage == VolEnvStage::idle) 
                continue;

            voice.vol_env.tick(sample_length_sec, this->params);
            double key_relative_to_a4 = ((double)voice.key) - 69.0; // nice
            double frequency = 440.0 * pow(2.0, key_relative_to_a4 / 12.0); // todo: non-440 hz tuning, pitch wheel, mod vibrato, microtonality
            double sample = 0.0;
            if (this->wave_type == WaveType::sine) {
                // todo: use a LUT
                sample = sin(time * frequency * 2.0 * 3.14159265); 
            }
            else if (this->wave_type == WaveType::square) {
                double wave_time = (time * frequency);
                double phase = wave_time - trunc(wave_time);
                double raw_sample = (phase < this->square_pulse_width) ? (+1.0) : (-1.0);
                raw_sample += poly_blep(phase, frequency * sample_length_sec);
                double t = phase - this->square_pulse_width;
                if (t < 0.0) {
                    t += 1.0;
                }
                raw_sample -= poly_blep(t, frequency * sample_length_sec);
                sample += raw_sample;
            }
            else if (this->wave_type == WaveType::triangle) {
                double wave_time = (time * frequency);
                double t_wrap = wave_time - trunc(wave_time);
                if (t_wrap < 0.5)   sample = (t_wrap * 4.0) - 1.0;
                else                sample = 1.0 - (t_wrap - 0.5) * 4.0; 
            }
            else if (this->wave_type == WaveType::sawtooth) {
                // todo: make this a weighted average, just like with gaussian filtering
                constexpr int n_samples = 8;
                const double delta = 0.25 * sample_length_sec;
                double t = time - (delta * (double)n_samples / 2.0);
                for (int j = 0; j < n_samples; ++j) {
                    t += delta;
                    double wave_time = (t * frequency);
                    double t_wrap = wave_time - trunc(wave_time);
                    sample += (t_wrap * 2.0) - 1.0;
                }
            }
            else if (this->wave_type== WaveType::noise) {
                uint32_t x = noise_state;
	            x ^= x << 13;
	            x ^= x >> 17;
	            x ^= x << 5;
                noise_state = x;
                sample = x / INT32_MAX;
            }
            double volume_multiplier = ((double)voice.velocity) / 127.0;
            double adsr_volume = voice.vol_env.adsr_volume;
            double final_volume = volume_multiplier * adsr_volume * Mixer::global_volume();
            final_volume = final_volume * final_volume;
            output[2*i + 0] += (float)(sample * final_volume);
            output[2*i + 1] += (float)(sample * final_volume);
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
