#include "scene.hpp"
#include <cassert>

namespace UI {
    void Pool::init(const size_t comp_size_) {
        comp_size = comp_size_;
        free(pool);
        pool = static_cast<uint8_t*>(malloc(MAX_ENTITIES * comp_size));
    }

    Pool::Pool(const size_t comp_size_) { init(comp_size_); }

    void* Pool::get(const size_t index) const {
        assert(pool != nullptr);
        return pool + index * comp_size;
    }
} // namespace UI
