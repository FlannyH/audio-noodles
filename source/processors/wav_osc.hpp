#pragma once

#include "../processor.hpp"
#include "../adsr.hpp"
#include <vector>

enum class WaveType {
    none = 0,
    sine,
    square,
    triangle,
    sawtooth,
    noise,
};

struct Voice {
    VolEnv vol_env;
    float actual_note;
    float velocity;
    float panning;
    double phase;
    uint8_t key;
};

struct WavOsc : Processor {
    WavOsc();
    void audio_process_block(const size_t n_frames, float* output) override;
    virtual void midi_note_on(int channel, uint8_t key, uint8_t velocity) override;
    virtual void midi_note_off(int channel, uint8_t key, uint8_t velocity) override;

    std::vector<Voice> voice_pool;
    VolEnvParams params;
    WaveType wave_type       = WaveType::sawtooth;
    double pitch_prev        = 0.0;
    float square_pulse_width = 0.375f;
    float unison_depth       = 0.3f;
    float unison_wideness    = 1.0f;
    float unison_phase_shift = 0.3f;
    int unison_count         = 9;
};
