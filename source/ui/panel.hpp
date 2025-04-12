#pragma once
#include <glm/vec2.hpp>
#include <glm/vec4.hpp>
#include "scene.hpp"

namespace UI {
    struct Panel {
        Scene scene;
        std::string name = "Yippee panels have names now :3";
        glm::vec2 top_left;
        glm::vec2 size;
        glm::vec2 pre_max_top_left;
        glm::vec2 pre_max_size;
        glm::vec2 begin_drag_mouse_pos;
        glm::vec4 bg_color = glm::vec4(0.1f, 0.1f, 0.2f, 1.0f);
        float double_click_timer = 0.0f;
        int resize_flags = 0;
        bool being_dragged = false;
        bool being_resized = false;
        bool maximized = false;

        void update(float delta_time);
    };
} // namespace UI
