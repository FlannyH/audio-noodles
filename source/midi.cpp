#include "midi.hpp"
#include "graph.hpp"
#include "log.hpp"
#include "track.hpp"
#include <RtMidi.h>
#include <memory>
#include <mutex>

namespace Midi {
    std::shared_ptr<RtMidiIn> midi_in;
    std::vector<MidiMessage> message_queue;
    std::vector<std::string> port_names;
    std::mutex message_queue_mutex;

    void midi_message_callback(double delta_time, std::vector<unsigned char>* message, void* user_data) {
        MidiMessage midi_message{};
        midi_message.status = message->at(0);
        if (message->size() > 1) midi_message.data1 = message->at(1);
        if (message->size() > 2) midi_message.data2 = message->at(2);
        if (message->size() > 3) midi_message.data3 = message->at(3);

        message_queue_mutex.lock();
        message_queue.push_back(midi_message);
        message_queue_mutex.unlock();
    }

    void init() {
        midi_in            = std::make_shared<RtMidiIn>();
        const auto n_ports = midi_in->getPortCount();

        if (n_ports == 0) {
            LOG(Warning, "No MIDI devices connected!");
            return;
        }

        port_names.resize(n_ports);
        for (unsigned int i = 0; i < n_ports; ++i) {
            LOG(Debug, "MIDI port %i: \"%s\"", i, midi_in->getPortName(i).c_str());
            port_names[i] = midi_in->getPortName(i);
        }

        midi_in->openPort(4);
        midi_in->setCallback(&midi_message_callback);

        LOG(Info, "MIDI device connected: \"%s\"", midi_in->getPortName(4).c_str());
    }

    void process() {
        if (message_queue.empty()) return;

        message_queue_mutex.lock();

        for (auto& message: message_queue) {
            const int type    = message.type();
            const int channel = message.channel();

            for (auto& track_p: Graph::get_tracks()) {
                std::shared_ptr<Track> track = std::static_pointer_cast<Track>(track_p);

                // If the track isn't listening to this midi channel, skip the track
                if ((track->midi_input_channel_mask & (1 << channel)) == 0) continue;

                if (type == 0) {
                    const uint8_t key      = message.data1;
                    const uint8_t velocity = message.data2;
                    track->midi_note_off(channel, key, velocity);
                } else if (type == 1) {
                    const uint8_t key      = message.data1;
                    const uint8_t velocity = message.data2;

                    if (velocity > 0) track->midi_note_on(channel, key, velocity);
                    else track->midi_note_off(channel, key, velocity);
                } else if (type == 2) {
                    const uint8_t key      = message.data1;
                    const uint8_t pressure = message.data2;
                    track->midi_poly_aftertouch(channel, key, pressure);
                } else if (type == 3) {
                    const uint8_t id    = message.data1;
                    const uint8_t value = message.data2;
                    track->midi_control_change(channel, id, value);
                } else if (type == 4) {
                    const uint8_t program = message.data1;
                    track->midi_program_change(channel, program);
                } else if (type == 5) {
                    const uint8_t pressure = message.data1;
                    track->midi_channel_aftertouch(channel, pressure);
                } else if (type == 6) {
                    const uint16_t value = message.data14();
                    track->midi_pitch_wheel(channel, value);
                }
            }
        }

        message_queue_mutex.unlock();

        message_queue.clear();
    }

    std::vector<std::string>& get_device_list() {
        return port_names;
    }
} // namespace Midi
