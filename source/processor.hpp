#pragma once
#include <cstdint>
#include <cstddef>

struct Processor {
    virtual void audio_process_block(const size_t n_samples, float* output) = 0;
    virtual void midi_note_on(int channel, uint8_t key, uint8_t velocity) {}
    virtual void midi_note_off(int channel, uint8_t key, uint8_t velocity) {}
    virtual void midi_poly_aftertouch(int channel, uint8_t key, uint8_t pressure) {}
    virtual void midi_control_change(int channel, uint8_t id, uint8_t value) {}
    virtual void midi_program_change(int channel, uint8_t program) {}
    virtual void midi_channel_aftertouch(int channel, uint8_t pressure) {}
    virtual void midi_pitch_wheel(int channel, uint16_t value) {}

    double pitch_wheel = 0.0;
    size_t ui_panel_index = -1;
};
