#include "track.hpp"
#include "components.hpp"
#include "log.hpp"
#include "midi.hpp"
#include "panel_manager.hpp"
#include "value_system.hpp"

Track::Track(size_t id) {
    // this->debug_processor = std::make_shared<WavOsc>();
    this->track_id = id;
    this->ui_panel_index = UI::load_panel("assets/layout/track.toml");

    // Find panel numberbox and set max range
    std::vector<std::string> devices = Midi::get_device_list();
    UI::Panel& panel = UI::get_panel(this->ui_panel_index);
    for (auto& entity : panel.scene.view<UI::Value, UI::NumberRange>()) {
        UI::Value* value = panel.scene.get_component<UI::Value>(entity);
        if (value->name != "midi_port") {
            continue;
        }
        UI::NumberRange* range = panel.scene.get_component<UI::NumberRange>(entity);
        if (range) {
            range->min = 0.0;
            range->max = devices.size() - 1;
        }
    }

    // Find panel combobox
    UI::Combobox* combobox = nullptr;
    for (auto& entity : panel.scene.view<UI::Combobox, UI::Value>()) {
        UI::Combobox* curr_combobox = panel.scene.get_component<UI::Combobox>(entity);
        UI::Value* value = panel.scene.get_component<UI::Value>(entity);
        if (value->name == "midi_port") {
            combobox = curr_combobox;
        }
    }
    if (!combobox) return;
    
    // Put MIDI device names in the panel's combobox
    combobox->list_items.resize(devices.size());
    for (size_t i = 0; i < devices.size(); ++i) {
        combobox->list_items[i] = std::wstring(devices[i].begin(), devices[i].end());
    }

    this->midi_device = std::make_shared<Midi::Device>(0);
}

void Track::midi_note_on(int channel, uint8_t key, uint8_t velocity) {
    LOG(Debug, "[Channel %2i] Note On: key %i, velocity %i", channel, key, velocity);
    // this->debug_processor->key_on(key, velocity);

    // Update debug text
    UI::Panel& panel = UI::get_panel(this->ui_panel_index);

    for (auto& entity : panel.scene.view<UI::Text, UI::Value>()) {
        auto* text = panel.scene.get_component<UI::Text>(entity);
        const auto* value = panel.scene.get_component<UI::Value>(entity);
        if (text && value->name == "elements.debug_text") {
            text->text = L"midi_on";
        }
    }
}

void Track::midi_note_off(int channel, uint8_t key, uint8_t velocity) {
    LOG(Debug, "[Channel %2i] Note Off: key %i, velocity %i", channel, key, velocity);
    // this->debug_processor->key_off(key);
}

void Track::midi_poly_aftertouch(int channel, uint8_t key, uint8_t pressure) {
    LOG(Debug, "[Channel %2i] Polyphonic Aftertouch: key %i, pressure %i", channel, key, pressure);
}

void Track::midi_control_change(int channel, uint8_t id, uint8_t value) {
    LOG(Debug, "[Channel %2i] Control Change: controller %i, data %i", channel, id, value);
}

void Track::midi_program_change(int channel, uint8_t program) {
    LOG(Debug, "[Channel %2i] Program Change: program %i", channel, program);
}

void Track::midi_channel_aftertouch(int channel, uint8_t pressure) {
    LOG(Debug, "[Channel %2i] Channel Aftertouch: pressure %i", channel, pressure);
}

void Track::midi_pitch_wheel(int channel, uint16_t value) { 
    const double value_normalized = ((double)value / 8192.0) - 1.0;
    const double value_in_range = value_normalized * this->pitch_wheel_range_cents;
    LOG(Debug, "[Channel %2i] Pitch Wheel: %4.2f cents", channel, value_in_range); 
    // this->debug_processor->set_pitch_wheel(value_in_range);
}

void Track::audio_process_block(const size_t n_samples, float* output) {
    if (this->track_id == -1) return;
    this->midi_device->process(this->track_id);

    UI::Panel& panel = UI::get_panel(this->ui_panel_index);
    const int target_port = (int)panel.scene.value_pool.get<double>("midi_port");

    if (this->midi_device->port != target_port) {
        this->midi_device.reset();
        this->midi_device = std::make_shared<Midi::Device>(target_port);
    }
}
