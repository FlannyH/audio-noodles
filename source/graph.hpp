#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>
#include <memory>

#include "processor.hpp"

namespace Graph {
    typedef uint32_t PortID;

    enum class LinkType {
        none = 0,
        midi = 1,
        audio = 2,
    };

    struct Link {
        PortID src_id;
        PortID dst_id;
        LinkType type;
    };
    
    size_t create_track();
    std::vector<std::shared_ptr<Processor>>& get_tracks();
    void dispatch();
} // namespace Graph
