#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace Midi {
    void init();
    void process();
    std::vector<std::string>& get_device_list();

    constexpr int midi_channel_global = -1;

    struct MidiMessage {
        uint8_t status;
        uint8_t data1;
        uint8_t data2;
        uint8_t data3;

        int channel() {
            if ((status & 0xF0) != 0xF0) return (status & 0x0F);
            else return midi_channel_global;
        }

        int type() { return (status >> 4) & 0x07; }

        uint16_t data14() { return (data2 << 7) + data1; }
    };
} // namespace Midi
