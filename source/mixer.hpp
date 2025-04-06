#pragma once
#include "processor.hpp"
#include <memory>

namespace Mixer {
    void init();
    void register_processor(std::shared_ptr<Processor> processor);
    double sample_rate();
    double block_start_time();
    double global_volume();
} // namespace Mixer
