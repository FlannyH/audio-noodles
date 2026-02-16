#include <vector>

#include "graph.hpp"
#include "track.hpp"

namespace Graph {
    std::vector<std::shared_ptr<Processor>> processors;
    std::vector<std::shared_ptr<Processor>> tracks;
    std::vector<Link> links;

    size_t create_track() {
        tracks.emplace_back(std::make_shared<Track>());
        return tracks.size() - 1;
    }

    std::vector<std::shared_ptr<Processor>>& get_tracks() { return tracks; }
}
