#pragma once
#include <glm/vec2.hpp>
#include <glm/vec4.hpp>
#include "scene.hpp"
#include "../graphics/resource.hpp"

namespace UI {
    struct Panel {
        Scene scene;
        std::string name = "Yippee panels have names now :3";
        glm::vec2 top_left;
        glm::vec2 min_size = {128.0f, 128.0f};
        glm::vec2 max_size = {99999.0f, 99999.0f};
        glm::vec2 size;
        glm::vec2 size_prev;
        glm::vec2 pre_max_top_left;
        glm::vec2 pre_max_size;
        glm::vec2 begin_drag_mouse_pos;
        glm::vec4 bg_color                    = glm::vec4(0.1f, 0.1f, 0.2f, 1.0f);
        Gfx::ResourceID content_render_target = Gfx::ResourceID::invalid();
        float double_click_timer              = 0.0f;
        int resize_flags                      = 0;
        bool being_dragged                    = false;
        bool being_resized                    = false;
        bool maximized                        = false;

        void update(float delta_time, bool do_mouse_interact);
        void render_window();
    };
    constexpr float window_bar_height  = 40.0f;
    constexpr float snap_sensitivity   = 24.0f;
    constexpr float unmax_distance     = 32.0f;
    constexpr float resize_sensitivity = 16.0f;
    constexpr float double_click_time  = 0.3f;
} // namespace UI
