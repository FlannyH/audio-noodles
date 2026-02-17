#pragma once
#include "RtMidi.h"
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace Midi {
    std::vector<std::string>& get_device_list(bool refresh = false);

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

    struct Device {
        std::shared_ptr<RtMidiIn> midi_in;
        std::vector<MidiMessage> message_queue;
        std::mutex message_queue_mutex;
        size_t port = 0;
        Device(size_t port = 0);
        void process(size_t track_id);
    };

    std::vector<std::string>& get_device_name_list();
} // namespace Midi
