#include "session.hpp"

namespace Session {
    struct {
        std::vector<Track> tracks;
    } data;

    size_t create_track() {
        data.tracks.emplace_back(Track{});
        return data.tracks.size() - 1;
    }

    std::vector<Track>& tracks() { return data.tracks; }
}; // namespace Session
