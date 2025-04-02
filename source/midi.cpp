#include <RtMidi.h>
#include "log.hpp"

namespace Midi {
    std::shared_ptr<RtMidiIn> midi_in;

    void midi_message_callback(double delta_time, std::vector<unsigned char>* message, void* user_data) {
        const uint8_t type = (message->at(0) >> 4) & 0x07;

        if (type == 0) {
            const uint8_t key = message->at(1);
            const uint8_t velocity = message->at(2);
            printf("Note Off: key %i, velocity %i\n", key, velocity);
        }
        else if (type == 1) {
            const uint8_t key = message->at(1);
            const uint8_t velocity = message->at(2);
            printf("Note On: key %i, velocity %i\n", key, velocity);
        }
        else if (type == 2) {
            const uint8_t key = message->at(1);
            const uint8_t pressure = message->at(2);
            printf("Polyphonic Aftertouch: key %i, pressure %i\n", key, pressure);
        }
        else if (type == 3) {
            const uint8_t controller = message->at(1);
            const uint8_t data = message->at(2);
            printf("Control Change: controller %i, data %i\n", controller, data);
        }
        else if (type == 4) {
            const uint8_t program = message->at(1);
            printf("Program Change: program %i\n", program);
        }
        else if (type == 5) {
            const uint8_t pressure = message->at(1);
            printf("Channel Aftertouch: pressure %i\n", pressure);
        }
        else if (type == 6) {
            const uint16_t lsb = message->at(1);
            const uint16_t msb = message->at(2);
            const uint16_t pitch_wheel = (msb << 8) + lsb;
            printf("Pitch Wheel: %i\n", pitch_wheel);
        }
    }

    void init() {
        midi_in = std::make_shared<RtMidiIn>();
        const auto n_ports = midi_in->getPortCount();

        if (n_ports == 0) {
            LOG(Warning, "No midi devices connected!");
            return;
        }

        midi_in->openPort(0);
        midi_in->setCallback(&midi_message_callback);
    }

    void process() {
    }
}
