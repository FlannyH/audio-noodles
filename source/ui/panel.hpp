#pragma once
#include <glm/vec2.hpp>
#include "scene.hpp"

namespace UI {
    struct Panel {
        Scene scene;
        glm::vec2 top_left;
        glm::vec2 size;

        void update(float delta_time);
    };
} // namespace UI
