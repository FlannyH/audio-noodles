#pragma once

#include <cstdint>
#include <memory>
#include "../midi.hpp"
#include "../processor.hpp"

struct Track : Processor {
    size_t track_id                  = SIZE_MAX;
    uint16_t midi_input_channel_mask = 1;
    double pitch_wheel_range_cents   = 200.0;
    std::shared_ptr<Midi::Device> midi_device;

    Track(size_t id);
    virtual void audio_process_block(const size_t n_samples, const double output_sample_rate, const Sample* input, Sample* output, std::vector<std::shared_ptr<Processor>> targets) override;
    virtual void midi_note_on(int channel, uint8_t key, uint8_t velocity) override;
    virtual void midi_note_off(int channel, uint8_t key, uint8_t velocity) override;
    virtual void midi_poly_aftertouch(int channel, uint8_t key, uint8_t pressure) override;
    virtual void midi_control_change(int channel, uint8_t id, uint8_t value) override;
    virtual void midi_program_change(int channel, uint8_t program) override;
    virtual void midi_channel_aftertouch(int channel, uint8_t pressure) override;
    virtual void midi_pitch_wheel(int channel, uint16_t value) override;
};
