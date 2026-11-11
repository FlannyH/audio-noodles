#include "wav_osc.hpp"
#include "../common.hpp"
#include "glm/ext/scalar_common.hpp"
#include "glm/ext/scalar_constants.hpp"
#include <ui/panel_manager.hpp>

#include <cmath>
#include <time.h>

thread_local uint32_t noise_state = 0x796C694C;

double poly_blep(double t, double dt) {
    if (t < dt) {
        t /= dt;
        return t + t - t * t - 1.0;
    } else if (t > 1.0 - dt) {
        t = (t - 1.0) / dt;
        return t * t + t + t + 1.0;
    }
    return 0.0;
}

WavOsc::WavOsc() {
    this->voice_pool.resize(2048);
    this->ui_panel_index = UI::load_panel("assets/layout/wav_osc.toml");
}

void WavOsc::audio_process_block(const size_t n_frames, const double output_sample_rate, const Sample* input, Sample* output, std::vector<std::shared_ptr<Processor>> targets) {
    const double sample_length_sec = 1.0 / output_sample_rate;

    if (this->ui_panel_index == -1) return;

    LOG(Info, "WavOsc::audio_process_block()");

    auto& panel              = UI::get_panel(this->ui_panel_index);
    this->square_pulse_width = (float)panel.scene.value_pool.get<double>("square_pulse_width");
    this->unison_depth       = (float)panel.scene.value_pool.get<double>("unison_depth");
    this->unison_wideness    = (float)panel.scene.value_pool.get<double>("unison_wideness");
    this->unison_phase_shift = (float)panel.scene.value_pool.get<double>("unison_phase_shift");
    this->unison_count       = (int)panel.scene.value_pool.get<double>("unison_count");

    this->params.delay   = panel.scene.value_pool.get<double>("adsr_delay");
    this->params.attack  = panel.scene.value_pool.get<double>("adsr_attack");
    this->params.hold    = panel.scene.value_pool.get<double>("adsr_hold");
    this->params.decay   = panel.scene.value_pool.get<double>("adsr_decay");
    this->params.sustain = panel.scene.value_pool.get<double>("adsr_sustain");
    this->params.release = 1.0 / panel.scene.value_pool.get<double>("adsr_release");

    const double pitch_wheel_start = this->pitch_prev;
    const double pitch_wheel_end   = this->pitch_wheel;

    for (size_t i = 0; i < n_frames; ++i) {
        // Smooth automation for pitch
        const double pitch_wheel_curr = std::lerp(pitch_wheel_start, pitch_wheel_end, (double)i / (double)n_frames);

        for (auto& voice: this->voice_pool) {
            if (voice.vol_env.stage == VolEnvStage::idle) continue;

            voice.vol_env.tick(sample_length_sec, this->params);
            const double key_relative_to_a4 = ((double)voice.actual_note) + (pitch_wheel_curr / 100.0) - 69.0; // nice
            const double frequency =
                440.0 * pow(2.0, key_relative_to_a4 / 12.0); // todo(wavosc_non_440hz): desc: non-440 hz tuning
                                                             // todo(wavosc_mod_vibrato): desc: vibrato
                                                             // todo(wavosc_microtonality): desc: microtonality
            double sample = 0.0;

            const double wave_time = voice.phase;

            if (this->wave_type == WaveType::sine) {
                // todo(wavosc_sine_lut): desc: use a LUT
                sample = sin(wave_time * 2.0 * 3.14159265);
            } else if (this->wave_type == WaveType::square) {
                const double phase = wave_time - trunc(wave_time);
                double raw_sample  = (phase < this->square_pulse_width) ? (+1.0) : (-1.0);
                raw_sample += poly_blep(phase, frequency * sample_length_sec);
                double t = phase - this->square_pulse_width;
                if (t < 0.0) t += 1.0;
                raw_sample -= poly_blep(t, frequency * sample_length_sec);
                sample += raw_sample;
            } else if (this->wave_type == WaveType::triangle) {
                const double t_wrap = wave_time - trunc(wave_time);
                if (t_wrap < 0.5) sample = (t_wrap * 4.0) - 1.0;
                else sample = 1.0 - (t_wrap - 0.5) * 4.0;
            } else if (this->wave_type == WaveType::sawtooth) {
                const double phase = wave_time - trunc(wave_time);
                double raw_sample  = (phase * 2.0) - 1.0;
                raw_sample -= poly_blep(phase, frequency * sample_length_sec);
                sample += raw_sample;
            } else if (this->wave_type == WaveType::noise) {
                uint32_t x = noise_state;
                x ^= x << 13;
                x ^= x >> 17;
                x ^= x << 5;
                noise_state = x;
                sample      = (((double)x / (double)UINT32_MAX) * 2.0) - 1.0;
            }
            const double volume_multiplier = (double)voice.velocity;
            const double adsr_volume       = voice.vol_env.adsr_volume;
            double final_volume            = volume_multiplier * adsr_volume;
            final_volume                   = final_volume * final_volume;
            output[i].left += (float)(sample * final_volume) *
                                 ((float)Common::lut_panning[0 + (size_t)((voice.panning + 1.0f) * 127.0)] / 4095.0f);
            output[i].right += (float)(sample * final_volume) *
                                 ((float)Common::lut_panning[254 - (size_t)((voice.panning + 1.0f) * 127.0)] / 4095.0f);
            voice.phase += sample_length_sec * frequency;
        }
    }

    pitch_prev = pitch_wheel;
}

void WavOsc::midi_note_on(int channel, uint8_t key, uint8_t velocity) {
    auto& panel     = UI::get_panel(this->ui_panel_index);
    this->wave_type = (WaveType)round(panel.scene.value_pool.get<double>("wave_type") + 1.0);
    LOG(Debug, "wave_type = %i", (int)this->wave_type);

    float key_center = (float)key;
    float phase_center = 0.0f;
    float pan_center = 0.0f;

    float key_range = (this->unison_depth / 2.0f);
    float phase_range = (this->unison_phase_shift / 2.0f);
    float pan_range = (this->unison_wideness / 2.0f);

    const float max_unison_i = (float)(this->unison_count - 1);

    for (int i = 0; i < this->unison_count; ++i) {
        float t_key   = 0.0f;
        float t_phase = 0.0f;
        float t_pan   = 0.0f;

        if (this->unison_count > 1) {
            float neg_to_pos = ((i / max_unison_i) * 2.0f) - 1.0f;
            float sign = (neg_to_pos >= 0.0f) ? +1.0f : -1.0f;
            float absolute = fabsf(neg_to_pos);
            t_key = powf(absolute * 0.5f * glm::pi<float>(), 0.5f) * sign;
            t_phase = t_key;
            t_pan = neg_to_pos;
        }

        for (auto& voice: this->voice_pool) {
            if (voice.vol_env.stage != VolEnvStage::idle) continue;

            float wrapped_phase      = glm::repeat(phase_center + (t_phase * phase_range));
            voice.phase              = wrapped_phase;
            voice.actual_note        = key_center + (t_key * key_range);
            voice.panning            = pan_center + (t_pan * pan_range);
            voice.key                = key;
            voice.velocity           = ((float)velocity / 127.0f) / sqrtf((float)this->unison_count);
            voice.vol_env.stage      = VolEnvStage::delay;
            voice.vol_env.stage_time = 0.0;
            break;
        }
    }
}

void WavOsc::midi_note_off(int channel, uint8_t key, uint8_t velocity) {
    (void)velocity;
    for (auto& voice: this->voice_pool) {
        if (voice.key == key) {
            voice.vol_env.stage = VolEnvStage::release;
        }
    }
}
