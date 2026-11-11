#include "output.hpp"
#include <components.hpp>
#include <ui/panel_manager.hpp>

int pa_callback(
    const void*, void* output_buffer, unsigned long frames_per_buffer, const PaStreamCallbackTimeInfo* time_info,
    PaStreamCallbackFlags flags, void* user_data) {
    Output* output_processor = (Output*)user_data;
    (void)flags;
    (void)time_info;

    LOG(Info, "Output::pa_callback() - output audio");

    if (output_processor->selected_index < 0) {
        memset(output_buffer, 0, sizeof(Sample) * frames_per_buffer);
        return paContinue;
    }

    output_processor->bridge_buffer_mtx.lock();
    {
        const PaDeviceInfo* info = Pa_GetDeviceInfo(output_processor->list_device_indices[output_processor->selected_index]);

        size_t n_to_copy = std::min(output_processor->bridge_buffer.size(), frames_per_buffer);

        if (n_to_copy == 0) {
            LOG(Warning, "Output buffer empty for %s", info->name);
        }
        else if (n_to_copy < frames_per_buffer) {
            LOG(Warning, "Output buffer underrun for %s", info->name);
        }

        for (size_t i = 0; i < n_to_copy; ++i) {
            ((Sample*)output_buffer)[i] = output_processor->bridge_buffer.front();
            output_processor->bridge_buffer.pop();
        }
    }
    output_processor->bridge_buffer_mtx.unlock();

    return paContinue;
}

static bool pa_initialized = false;

Output::Output(size_t id) {
    if (!pa_initialized) {
        Pa_Initialize();
    }

    this->ui_panel_index = UI::load_panel("assets/layout/output.toml");
    UI::Panel& panel = UI::get_panel(this->ui_panel_index);

    // Find panel combobox
    UI::Combobox* combobox = nullptr;
    for (auto& entity: panel.scene.view<UI::Combobox, UI::Value>()) {
        UI::Combobox* curr_combobox = panel.scene.get_component<UI::Combobox>(entity);
        UI::Value* value            = panel.scene.get_component<UI::Value>(entity);
        if (value->name == "output_port") {
            combobox = curr_combobox;
        }
    }
    if (!combobox) return;

    // Put output device names in the panel's combobox
    combobox->list_items.clear();
    combobox->list_items.reserve(Pa_GetDeviceCount());
    this->list_device_indices.reserve(Pa_GetDeviceCount());

    for (size_t i = 0; i < Pa_GetDeviceCount(); ++i) {
        const PaDeviceInfo* info = Pa_GetDeviceInfo((PaDeviceIndex)i);
        std::string name_string  = info->name;
        if (info->maxOutputChannels < 2) continue;
        combobox->list_items.push_back(std::wstring(name_string.begin(), name_string.end()));
        this->list_device_indices.push_back(i);
    }

    if (this->list_device_indices.empty()) {
        combobox->list_items.push_back(L"(no devices found)");
        return;
    }

    this->max_index = combobox->list_items.size() - 1;

    // Find panel numberbox and set max range
    for (auto& entity: panel.scene.view<UI::Value, UI::NumberRange>()) {
        UI::Value* value = panel.scene.get_component<UI::Value>(entity);
        if (value->name != "output_port") {
            continue;
        }
        UI::NumberRange* range = panel.scene.get_component<UI::NumberRange>(entity);
        if (range) {
            range->min = 0.0;
            range->max = combobox->list_items.size() - 1.0;
        }
    }

    if (combobox->current_selected_index < 0) return;
    if (combobox->current_selected_index >= this->list_device_indices.size()) return;

    const PaDeviceIndex dev_i                  = this->list_device_indices[combobox->current_selected_index];
    const PaStreamParameters output_parameters = {dev_i, 2, paFloat32, Pa_GetDeviceInfo(dev_i)->defaultLowOutputLatency, NULL};
    if (!Pa_OpenStream(&this->stream, NULL, &output_parameters, output_sample_rate, paFramesPerBufferUnspecified, paClipOff, &pa_callback, this)) {
        Pa_StartStream(this->stream);
    }
}

void Output::audio_process_block(
    const size_t n_samples, const double output_sample_rate, const Sample* input, Sample* output,
    std::vector<std::shared_ptr<Processor>> targets) {
    if (this->ui_panel_index == -1) return;

    LOG(Info, "Output::audio_process_block() - copy into bridge_buffer");

    UI::ValuePool& value_pool = UI::get_panel(this->ui_panel_index).scene.value_pool;

    int output_port = (int)value_pool.get<double>("output_port");
    if (output_port > this->max_index) {
        output_port = this->max_index;
        value_pool.set_value<double>("output_port", output_port);
    }

    // handle output device switching
    if (this->selected_index != output_port) {
        PaError error{};
        error = Pa_CloseStream(this->stream);
        printf("%i ", error);
        const int output_device                    = this->list_device_indices[output_port];
        const PaStreamParameters output_parameters = {
            output_device, 2, paFloat32, Pa_GetDeviceInfo((int)output_port)->defaultLowOutputLatency, NULL,
        };
        if (!(error = Pa_OpenStream(&this->stream, NULL, &output_parameters, output_sample_rate, paFramesPerBufferUnspecified, paNoFlag, &pa_callback, this))) {
            error = Pa_StartStream(this->stream);
        }
        (void)error; // it's just here for debugging

        this->selected_index = output_port;
    }

    auto& panel     = UI::get_panel(this->ui_panel_index);
    auto& input_pin = panel.pins["audio_in"];

    this->bridge_buffer_mtx.lock();
    {
        if (input) {
            for (size_t i = 0; i < n_samples; ++i) {
                this->bridge_buffer.push(input[i]);
            }
        } else {
            for (size_t i = 0; i < n_samples; ++i) {
                this->bridge_buffer.push({0.0, 0.0});
            }
        }
    }
    this->bridge_buffer_mtx.unlock();
}
