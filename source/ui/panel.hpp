#pragma once
#include <glm/vec2.hpp>
#include <glm/vec4.hpp>
#include "scene.hpp"

namespace UI {
    struct Panel {
        Scene scene;
        glm::vec2 top_left;
        glm::vec2 size;
        glm::vec4 bg_color = glm::vec4(0.1f, 0.1f, 0.2f, 1.0f);

        void update(float delta_time);
    };
} // namespace UI
