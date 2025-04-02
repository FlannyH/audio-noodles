#pragma once
#include <cstdint>

namespace Midi {
    void init();
    void process();
    
    constexpr int midi_channel_global = -1;

    struct MidiMessage {
        uint8_t status;
        uint8_t data1;
        uint8_t data2;
        uint8_t data3;

        int channel() {
            if ((status & 0xF0) != 0xF0) 
                return (status & 0x0F);
            else
                return midi_channel_global; 
        }

        int type() {
            return (status >> 4) & 0x07;
        }

        uint16_t data16() {
            return (data2 << 8) + data1;
        }
    };
}
