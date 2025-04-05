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
    this->voice_pool.resize(128);
}

void WavOsc::process_block(const size_t n_frames, float *output) {
    const double sample_length_sec = 1.0 / Mixer::sample_rate();

    for (size_t i = 0; i < n_frames; ++i) {
        for (auto& voice : this->voice_pool) {
            if (voice.vol_env.stage == VolEnvStage::idle) 
                continue;

            voice.vol_env.tick(sample_length_sec, this->params);
            const double key_relative_to_a4 = ((double)voice.actual_note) - 69.0; // nice
            const double frequency = 440.0 * pow(2.0, key_relative_to_a4 / 12.0); // todo: non-440 hz tuning, pitch wheel, mod vibrato, microtonality
            double sample = 0.0;
            
            if (this->wave_type == WaveType::sine) {
                // todo: use a LUT
                sample = sin(voice.phase * frequency * 2.0 * 3.14159265); 
            }
            else if (this->wave_type == WaveType::square) {
                const double wave_time = (voice.phase * frequency);
                const double phase = wave_time - trunc(wave_time);
                double raw_sample = (phase < this->square_pulse_width) ? (+1.0) : (-1.0);
                raw_sample += poly_blep(phase, frequency * sample_length_sec);
                double t = phase - this->square_pulse_width;
                if (t < 0.0) t += 1.0;
                raw_sample -= poly_blep(t, frequency * sample_length_sec);
                sample += raw_sample;
            }
            else if (this->wave_type == WaveType::triangle) {
                const double wave_time = (voice.phase * frequency);
                const double t_wrap = wave_time - trunc(wave_time);
                if (t_wrap < 0.5)   sample = (t_wrap * 4.0) - 1.0;
                else                sample = 1.0 - (t_wrap - 0.5) * 4.0; 
            }
            else if (this->wave_type == WaveType::sawtooth) {
                const double wave_time = (voice.phase * frequency);
                const double phase = wave_time - trunc(wave_time);
                double raw_sample = (phase * 2.0) - 1.0;
                raw_sample -= poly_blep(phase, frequency * sample_length_sec);
                sample += raw_sample;
            }
            else if (this->wave_type== WaveType::noise) {
                uint32_t x = noise_state;
	            x ^= x << 13;
	            x ^= x >> 17;
	            x ^= x << 5;
                noise_state = x;
                sample = x / INT32_MAX;
            }
            const double volume_multiplier = (double)voice.velocity;
            const double adsr_volume = voice.vol_env.adsr_volume;
            double final_volume = volume_multiplier * adsr_volume * Mixer::global_volume();
            final_volume = final_volume * final_volume;
            output[2*i + 0] += (float)(sample * final_volume) * ((float)Common::lut_panning[0   + (size_t)((voice.panning + 1.0f) * 127.0)] / 4095.0f);
            output[2*i + 1] += (float)(sample * final_volume) * ((float)Common::lut_panning[254 - (size_t)((voice.panning + 1.0f) * 127.0)] / 4095.0f);
            voice.phase += sample_length_sec;
        }
    }
}

void WavOsc::key_on(uint8_t key, uint8_t velocity) {
    float fkey = (float)key - (this->unison_depth / 2.0f);
    float fphase = -this->unison_phase_shift / 2.0f;
    float fpan = -this->unison_wideness;
    const float key_delta = this->unison_depth / (float)(this->unison_count - 1);
    const float phase_delta = this->unison_phase_shift / (float)(this->unison_count - 1);
    const float pan_delta = this->unison_wideness * 2.0f / (float)(this->unison_count - 1);
    for (int i = 0; i < this->unison_count; ++i) {
        for (auto& voice : this->voice_pool) {
            if (voice.vol_env.stage != VolEnvStage::idle) 
                continue;

            float wrapped_phase = (fphase < 0.0f) ? (fphase + 1.0f) : (fphase);
            wrapped_phase = (wrapped_phase >= 1.0f) ? (wrapped_phase - 1.0f) : (wrapped_phase);
            voice.phase = wrapped_phase;
            voice.actual_note = fkey;
            voice.panning = fpan;
            voice.key = key;
            voice.velocity = ((float)velocity / 127.0f) / sqrtf((float)this->unison_count);
            voice.vol_env.stage = VolEnvStage::delay;
            break;
        }
        fkey += key_delta;
        fphase += phase_delta;
        fpan += pan_delta;
    }
}

void WavOsc::key_off(uint8_t key) {
    for (auto& voice : this->voice_pool) {
        if (voice.key == key) {
            voice.vol_env.stage = VolEnvStage::release;
        }
    }
}
