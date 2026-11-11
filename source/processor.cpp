#include "processor.hpp"
#include "panel_manager.hpp"

void Processor::consume_midi(std::vector<Midi::MidiMessage> messages) {
    LOG(Info, "Processor(%s)::consume_midi(...)", UI::get_panel(this->ui_panel_index).title.c_str());

    for (auto& message: messages) {
        const int type    = message.type();
        const int channel = message.channel();

        if (type == 0) {
            const uint8_t key      = message.data1;
            const uint8_t velocity = message.data2;
            this->midi_note_off(channel, key, velocity);
        }
        else if (type == 1) {
            const uint8_t key      = message.data1;
            const uint8_t velocity = message.data2;
            if (velocity > 0) this->midi_note_on(channel, key, velocity);
            else this->midi_note_off(channel, key, velocity);
        }
        else if (type == 2) {
            const uint8_t key      = message.data1;
            const uint8_t pressure = message.data2;
            this->midi_poly_aftertouch(channel, key, pressure);
        }
        else if (type == 3) {
            const uint8_t id    = message.data1;
            const uint8_t value = message.data2;
            this->midi_control_change(channel, id, value);
        }
        else if (type == 4) {
            const uint8_t program = message.data1;
            this->midi_program_change(channel, program);
        }
        else if (type == 5) {
            const uint8_t pressure = message.data1;
            this->midi_channel_aftertouch(channel, pressure);
        }
        else if (type == 6) {
            const uint16_t value = message.data14();
            this->midi_pitch_wheel(channel, value);
        }
    }
}
