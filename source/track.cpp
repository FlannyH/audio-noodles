#include "track.hpp"
#include "log.hpp"

void Track::midi_note_on(int channel, uint8_t key, uint8_t velocity) {
    LOG(Debug, "[Channel %2i] Note On: key %i, velocity %i", channel, key, velocity);
}

void Track::midi_note_off(int channel, uint8_t key, uint8_t velocity) {
    LOG(Debug, "[Channel %2i] Note Off: key %i, velocity %i", channel, key, velocity);
}

void Track::midi_poly_aftertouch(int channel, uint8_t key, uint8_t pressure) {
    LOG(Debug, "[Channel %2i] Polyphonic Aftertouch: key %i, pressure %i", channel, key, pressure);
}

void Track::midi_control_change(int channel, uint8_t id, uint8_t value) {
    LOG(Debug, "[Channel %2i] Control Change: controller %i, data %i", channel, id, value);
}

void Track::midi_program_change(int channel, uint8_t program) {
    LOG(Debug, "[Channel %2i] Program Change: program %i", channel, program);
}

void Track::midi_channel_aftertouch(int channel, uint8_t pressure) {
    LOG(Debug, "[Channel %2i] Channel Aftertouch: pressure %i", channel, pressure);
}

void Track::midi_pitch_wheel(int channel, uint16_t value) {
    LOG(Debug, "[Channel %2i] Pitch Wheel: %i", channel, value);
}

