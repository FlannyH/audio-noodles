#include <RtMidi.h>
#include <memory>
#include <mutex>
#include <log.hpp>

#include "midi.hpp"
#include "graph.hpp"
#include "processors/track.hpp"

namespace Midi {
    thread_local std::vector<std::string> port_names;
    thread_local uint32_t port_name_version = 0;

    std::vector<std::string>& get_device_list(bool refresh) {
        if (port_name_version > 0 && !refresh) {
            return port_names;
        }

        LOG(Debug, "Fetching MIDI port names");

        auto midi_in       = std::make_shared<RtMidiIn>();
        const auto n_ports = midi_in->getPortCount();

        if (n_ports == 0) {
            LOG(Info, "No MIDI devices connected");
            ++port_name_version;
            return port_names;
        }

        port_names.resize(n_ports);
        for (unsigned int i = 0; i < n_ports; ++i) {
            LOG(Debug, "MIDI port %i: \"%s\"", i, midi_in->getPortName(i).c_str());
            port_names[i] = midi_in->getPortName(i);
        }

        ++port_name_version;
        return port_names;
    }

    void midi_message_callback(double delta_time, std::vector<unsigned char>* message, void* user_data) {
        LOG(Debug, "%.3f", delta_time);
        MidiMessage midi_message{};
        midi_message.status = message->at(0);
        if (message->size() > 1) midi_message.data1 = message->at(1);
        if (message->size() > 2) midi_message.data2 = message->at(2);
        if (message->size() > 3) midi_message.data3 = message->at(3);

        Device* device = (Device*)user_data;
        device->message_queue_mutex.lock();
        device->message_queue.push_back(midi_message);
        device->message_queue_mutex.unlock();
    }

    Device::Device(size_t port) {
        this->port = port;
        get_device_list(true);

        this->midi_in = std::make_shared<RtMidiIn>();
        if (this->midi_in->getPortCount() > port) {
            this->midi_in->openPort((unsigned int)port);
            this->midi_in->setCallback(&midi_message_callback, this);

            LOG(Info, "MIDI device connected: \"%s\"", midi_in->getPortName((unsigned int)port).c_str());
        }
    }

    std::vector<MidiMessage> Device::pull_messages(size_t track_id) {
        auto track_p                 = Graph::get_tracks().at(track_id);
        std::shared_ptr<Track> track = std::static_pointer_cast<Track>(track_p);

        
        this->message_queue_mutex.lock();
        auto message_queue_copy = std::move(this->message_queue);
        this->message_queue_mutex.unlock();

        return message_queue_copy;
    }
} // namespace Midi
