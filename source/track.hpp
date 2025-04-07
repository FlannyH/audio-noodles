#pragma once
#include <cstdint>
#include <memory>
#include "processors/wav_osc.hpp"

struct Track {
    uint16_t midi_input_channel_mask        = 1;
    double pitch_wheel_range_cents          = 200.0;
    std::shared_ptr<WavOsc> debug_processor = nullptr;

    Track();
    void midi_note_on(int channel, uint8_t key, uint8_t velocity);
    void midi_note_off(int channel, uint8_t key, uint8_t velocity);
    void midi_poly_aftertouch(int channel, uint8_t key, uint8_t pressure);
    void midi_control_change(int channel, uint8_t id, uint8_t value);
    void midi_program_change(int channel, uint8_t program);
    void midi_channel_aftertouch(int channel, uint8_t pressure);
    void midi_pitch_wheel(int channel, uint16_t value);
};
