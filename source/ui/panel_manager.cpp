#include "panel_manager.hpp"
#include "../graphics/renderer.hpp"
#include "components.hpp"
#include <stdexcept>

#define MAX_PANEL_COUNT 256

namespace UI {
    Panel panel_pool[MAX_PANEL_COUNT]{};
    bool panel_allocated[MAX_PANEL_COUNT] = {false};
    std::vector<size_t> panel_order;

    Panel& new_panel(PanelCreateInfo&& panel_create_info) {
        for (size_t i = 0; i < MAX_PANEL_COUNT; ++i) {
            if (panel_allocated[i] == false) {
                panel_allocated[i] = true;
                panel_order.push_back(i);
                panel_pool[i].scene              = Scene();
                panel_pool[i].name               = panel_create_info.name;
                panel_pool[i].top_left           = panel_create_info.top_left;
                panel_pool[i].min_size           = panel_create_info.min_size;
                panel_pool[i].max_size           = panel_create_info.max_size;
                panel_pool[i].size               = panel_create_info.size;
                panel_pool[i].size_prev          = {0, 0};
                panel_pool[i].pre_max_top_left   = panel_create_info.top_left;
                panel_pool[i].pre_max_size       = panel_create_info.size;
                panel_pool[i].bg_color           = panel_create_info.bg_color;
                panel_pool[i].double_click_timer = 0.0f;
                panel_pool[i].resize_flags       = 0;
                panel_pool[i].being_dragged      = false;
                panel_pool[i].being_resized      = false;
                panel_pool[i].maximized          = panel_create_info.maximized;
                return panel_pool[i];
            }
        }
        std::runtime_error("Ran out of UI panels!");
        return panel_pool[0]; // we can't get here so this one's just for the compiler, as a treat :3
    }

    void panel_input() {
        for (const auto& index: panel_order) {
            const auto& panel   = panel_pool[index];
            Hitbox panel_hitbox = {.top_left = panel.top_left, .bottom_right = panel.top_left + panel.size};
            if (panel_hitbox.intersects(Input::mouse_position_pixels())) {
                panel_pool[index].update(Gfx::get_delta_time());
                break;
            }
        }
    }

    void panel_render() {
        for (size_t i = panel_order.size(); i-- > 0;) {
            printf("rendering %i\n", i);
            panel_pool[panel_order[i]].render_window();
        }
    }
} // namespace UI
