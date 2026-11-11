#pragma once

#include <mutex>
#include <memory>
#include <queue>
#include "../processor.hpp"
#include "portaudio.h"

struct Output : Processor {
    PaStream* stream                = NULL;
    double output_sample_rate       = 44100; // Hz
    double block_start_time_value   = 0.0; // sec
    double global_volume_value      = 0.8;
    int selected_index = -1; // -1 means none
    int max_index = 0;
    std::mutex bridge_buffer_mtx;
    std::queue<Sample> bridge_buffer; // samples done rendering, waiting to be output
    std::vector<PaDeviceIndex> list_device_indices;

    Output(size_t id);
    virtual void audio_process_block(const size_t n_samples, const double output_sample_rate, const Sample* input, Sample* output, std::vector<std::shared_ptr<Processor>> targets) override;
};
