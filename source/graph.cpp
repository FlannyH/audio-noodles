#include <vector>

#include "graph.hpp"
#include "track.hpp"

namespace Graph {
    std::vector<std::shared_ptr<Processor>> processors;
    std::vector<std::shared_ptr<Processor>> tracks;
    std::vector<Link> links;

    size_t create_track() {
        tracks.emplace_back(std::make_shared<Track>(tracks.size()));
        return tracks.size() - 1;
    }

    std::vector<std::shared_ptr<Processor>>& get_tracks() { return tracks; }

    void dispatch() {
        // todo(graph_dispatch): desc: actually execute the graph insteadof just executing the first one
        tracks[0]->audio_process_block(0, nullptr);
    }
} // namespace Graph
