#pragma once
#include <cstdint>
#include <cstddef>
#include <memory>
#include <vector>
#include "common.hpp"
#include "midi.hpp"

struct Processor {
    virtual void audio_process_block(const size_t n_samples, const double output_sample_rate, const Sample* input, Sample* output, std::vector<std::shared_ptr<Processor>> targets) = 0;
    virtual void midi_note_on(int channel, uint8_t key, uint8_t velocity) {}
    virtual void midi_note_off(int channel, uint8_t key, uint8_t velocity) {}
    virtual void midi_poly_aftertouch(int channel, uint8_t key, uint8_t pressure) {}
    virtual void midi_control_change(int channel, uint8_t id, uint8_t value) {}
    virtual void midi_program_change(int channel, uint8_t program) {}
    virtual void midi_channel_aftertouch(int channel, uint8_t pressure) {}
    virtual void midi_pitch_wheel(int channel, uint16_t value) {}
    void consume_midi(std::vector<Midi::MidiMessage> messages);

    double pitch_wheel    = 0.0;
    int ui_panel_index = -1;
    int output_buffer_offset = -1;
    static std::vector<Sample> sample_pool;
};
