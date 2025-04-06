#pragma once
#include "track.hpp"
#include <vector>

namespace Session {
    size_t create_track();
    std::vector<Track>& tracks();
} // namespace Session