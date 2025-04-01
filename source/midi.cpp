#include <RtMidi.h>
#include "log.hpp"

namespace Midi {
    std::shared_ptr<RtMidiIn> midi_in;

    void midi_message_callback(double delta_time, std::vector<unsigned char>* message, void* user_data) {
        for (size_t i = 0; i < message->size(); ++i) {
            printf ("%02X ", message->at(i));
        }
        printf("\n");
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
