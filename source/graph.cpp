#include <alsa/asoundlib.h>
#include <vector>

#include <ui/panel_manager.hpp>
#include "graph.hpp"
#include "processor.hpp"
#include "processors/output.hpp"
#include "processors/wav_osc.hpp"
#include "processors/track.hpp"

namespace Graph {
    struct ScheduleEntry {
        std::shared_ptr<Processor> processor;
        std::vector<std::shared_ptr<Processor>> inputs;
        std::vector<std::shared_ptr<Processor>> outputs;
    };

    std::vector<std::shared_ptr<Processor>> processors; // all processors
    std::vector<std::shared_ptr<Processor>> tracks; // only track processors
    std::vector<std::shared_ptr<Processor>> outputs; // only output processors
    std::vector<ScheduleEntry> curr_schedule; // what will be executed next dispatch(), in order
    std::vector<Sample> working_buffers;

    const size_t curr_sample_rate = 44100;
    const size_t default_buffer_size = 1024;
    size_t buffer_size = default_buffer_size;

    size_t create_track() {
        auto track = std::make_shared<Track>(tracks.size());
        auto wav_osc = std::make_shared<WavOsc>();
        auto wav_osc2 = std::make_shared<WavOsc>();
        processors.emplace_back(track);
        processors.emplace_back(wav_osc);
        processors.emplace_back(wav_osc2);
        tracks.emplace_back(track);
        return tracks.size() - 1;
    }

    size_t create_output() {
        auto output = std::make_shared<Output>(outputs.size());
        outputs.emplace_back(output);
        processors.emplace_back(output);
        return outputs.size() - 1;
        // return 0;
    }

    std::vector<std::shared_ptr<Processor>>& get_tracks() { return tracks; }

    void schedule() {
        const std::vector<UI::Noodle>& noodles = UI::get_noodles();
        const std::vector<UI::Panel*> panels = UI::get_panels_in_order();

        curr_schedule.clear();

        std::vector<bool> ready_to_go(processors.size());
        std::vector<bool> visit_list(processors.size());

        // Mark all processors as ready unless they have populated input pins on their panels
        int changes = 0;
        do {
            changes = 0;

            // Check all processors to know which ones to dispatch next
            for (size_t i = 0; i < processors.size(); ++i) {
                if (visit_list[i]) continue;

                const auto& panel = UI::get_panel(processors[i]->ui_panel_index);

                bool ready = true;

                std::vector<std::shared_ptr<Processor>> curr_targets;
                std::vector<std::shared_ptr<Processor>> curr_inputs;

                // Check all the current panel's pins
                for (auto& [name, pin] : panel.pins) {
                    // If this panel has an input pin
                    if (pin.direction == UI::PinDirection::Input) {
                        // Find noodle that has this input pin as a destination
                        for (auto& noodle : noodles) {
                            if (noodle.destination.panel_id != pin.panel_id) continue;
                            if (noodle.destination.pin_id != pin.pin_id) continue;

                            // Find the processor corresponding to the noodle source's panel id
                            for (size_t i2 = 0; i2 < processors.size(); ++i2) {
                                if (i == i2) continue;
                                if (processors[i2]->ui_panel_index != noodle.source.panel_id) continue;
                                curr_inputs.push_back(processors[i2]);
                                // If that processor is already in the schedule before the current
                                // processor, the current processor is ready to go too
                                ready &= visit_list[i2];
                            }
                        }
                    }
                    else if (pin.direction == UI::PinDirection::Output) {
                        // Find noodle that has this output pin as a source
                        for (auto& noodle : noodles) {
                            if (noodle.source.panel_id != pin.panel_id) continue;
                            if (noodle.source.pin_id != pin.pin_id) continue;

                            // Find the processor corresponding to the noodle destination's panel id
                            for (size_t i2 = 0; i2 < processors.size(); ++i2) {
                                if (i == i2) continue;
                                if (processors[i2]->ui_panel_index != noodle.destination.panel_id) continue;
                                curr_targets.push_back(processors[i2]);
                            }
                        }
                    }
                }

                // Add ready to go processors to the schedule
                if(ready && !ready_to_go[i]) {
                    curr_schedule.push_back({
                        processors[i],
                        curr_inputs,
                        curr_targets,
                    });
                    visit_list[i] = true;
                    ready_to_go[i] = true;
                    changes++;
                }
            }
        } while (changes);

        return;
    }

    void dispatch(size_t sample_count) {
        working_buffers.clear();

        // alloc buffers
        for (auto& processor : processors) {
            processor->output_buffer_offset = working_buffers.size();
            working_buffers.resize(working_buffers.size() + sample_count);;
        }

        LOG(Info, "Dispatch (%i samples)", (int)sample_count);
        for (auto& job : curr_schedule) {
            LOG(Info, "- %s", UI::get_panel(job.processor->ui_panel_index).title.c_str());
            Sample* input_buffer = nullptr;
            Sample* output_buffer = nullptr;

            if (!job.inputs.empty()) {
                // let's pick the first one to be the accumulator
                Sample* accumulator = &working_buffers[job.inputs[0]->output_buffer_offset];
                for (size_t input_i = 1; input_i < job.inputs.size(); ++input_i) {
                    const Sample* buffer = &working_buffers[job.inputs[input_i]->output_buffer_offset];

                    for (size_t i = 1; i < sample_count; ++i) {
                        accumulator[i].left += buffer[i].left;
                        accumulator[i].right += buffer[i].right;
                    }
                }
                input_buffer = accumulator;
            }

            if (job.processor->output_buffer_offset != -1) {
                output_buffer = &working_buffers[job.processor->output_buffer_offset];
            }

            job.processor->audio_process_block(sample_count, curr_sample_rate, input_buffer, output_buffer, job.outputs);
        }
    }

    void run() {
        // determine health of all the buffers
        size_t min_ready = SIZE_MAX;
        size_t max_ready = 0;

        for (const auto& output: outputs) {
            const size_t samples_ready = std::static_pointer_cast<Output>(output)->bridge_buffer.size();
            if (samples_ready < min_ready) min_ready = samples_ready;
            if (samples_ready > max_ready) max_ready = samples_ready;
        }

        // min_ready will never reach size_max so if we do, it means no outputs found
        if (min_ready == SIZE_MAX) return;

        if (min_ready < buffer_size) {
            schedule();
            dispatch(buffer_size);
        }
    }
} // namespace Graph
