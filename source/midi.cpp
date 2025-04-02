#include "midi.hpp"
#include "log.hpp"
#include "session.hpp"
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

            for (auto& track : Session::tracks()) {     
                // If the track isn't listening to this midi channel, skip the track
                if ((track.midi_input_channel_mask & (1 << channel)) == 0)
                    continue;

                if (type == 0) {
                    const uint8_t key = message.data1;
                    const uint8_t velocity = message.data2;
                    track.midi_note_off(channel, key, velocity);
                }
                else if (type == 1) {
                    const uint8_t key = message.data1;
                    const uint8_t velocity = message.data2;
                    
                    if (velocity > 0) 
                        track.midi_note_on(channel, key, velocity);
                    else 
                        track.midi_note_off(channel, key, velocity);
                }
                else if (type == 2) {
                    const uint8_t key = message.data1;
                    const uint8_t pressure = message.data2;
                    track.midi_poly_aftertouch(channel, key, pressure);
                }
                else if (type == 3) {
                    const uint8_t id = message.data1;
                    const uint8_t value = message.data2;
                    track.midi_control_change(channel, id, value);
                }
                else if (type == 4) {
                    const uint8_t program = message.data1;
                    track.midi_program_change(channel, program);
                }
                else if (type == 5) {
                    const uint8_t pressure = message.data1;
                    track.midi_channel_aftertouch(channel, pressure);
                }
                else if (type == 6) {
                    const uint16_t value = message.data16();
                    track.midi_pitch_wheel(channel, value);
                }
            }
        }

        mutex.unlock();

        message_queue.clear();
    }
}
