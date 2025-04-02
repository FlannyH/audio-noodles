#include "midi.hpp"
#include "log.hpp"
#include <RtMidi.h>
#include <mutex>

namespace Midi {
    std::shared_ptr<RtMidiIn> midi_in;
    std::vector<MidiMessage> message_queue;
    std::mutex mutex;

    void midi_message_callback(double delta_time, std::vector<unsigned char>* message, void* user_data) {
        MidiMessage midi_message{};
        midi_message.status = message->at(0);
        if (message->size() > 1) 
            midi_message.data1 = message->at(1);
        if (message->size() > 2) 
            midi_message.data2 = message->at(2);
        if (message->size() > 3) 
            midi_message.data3 = message->at(3);

        mutex.lock();
        message_queue.push_back(midi_message);
        mutex.unlock();
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
        if (message_queue.empty()) return;

        mutex.lock();

        for (auto& message : message_queue) {
            const int type = message.type();
            const int channel = message.channel();

            if (type == 0) {
                const uint8_t key = message.data1;
                const uint8_t velocity = message.data2;
                printf("[Channel %2i] Note Off: key %i, velocity %i\n", channel, key, velocity);
            }
            else if (type == 1) {
                const uint8_t key = message.data1;
                const uint8_t velocity = message.data2;
                printf("[Channel %2i] Note On: key %i, velocity %i\n", channel, key, velocity);
            }
            else if (type == 2) {
                const uint8_t key = message.data1;
                const uint8_t pressure = message.data2;
                printf("[Channel %2i] Polyphonic Aftertouch: key %i, pressure %i\n", channel, key, pressure);
            }
            else if (type == 3) {
                const uint8_t controller = message.data1;
                const uint8_t data = message.data2;
                printf("[Channel %2i] Control Change: controller %i, data %i\n", channel, controller, data);
            }
            else if (type == 4) {
                const uint8_t program = message.data1;
                printf("[Channel %2i] Program Change: program %i\n", channel, program);
            }
            else if (type == 5) {
                const uint8_t pressure = message.data1;
                printf("[Channel %2i] Channel Aftertouch: pressure %i\n", channel, pressure);
            }
            else if (type == 6) {
                const uint16_t pitch_wheel = message.data16();
                printf("[Channel %2i] Pitch Wheel: %i\n", channel, pitch_wheel);
            }
        }

        mutex.unlock();

        message_queue.clear();
    }
}
