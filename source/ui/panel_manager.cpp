#include "panel_manager.hpp"
#include "../graphics/renderer.hpp"
#include "components.hpp"
#include <stdexcept>
#include <toml++/toml.hpp>
#include <iostream>

#define MAX_PANEL_COUNT 256

namespace UI {
    Panel panel_pool[MAX_PANEL_COUNT]{};
    bool panel_allocated[MAX_PANEL_COUNT] = {false};
    std::vector<size_t> panel_order;
    std::vector<size_t> panel_order_scratch;
    std::vector<Panel*> panels;
    size_t prev_panel_to_focus_on = -1;
    bool panels_dirty             = false;

    std::vector<Panel*>& get_panels() {
        if (panels_dirty) {
            panels.clear();
            for (const auto& index: panel_order) {
                panels.push_back(&panel_pool[index]);
            }
        }
        return panels;
    }

    Panel& new_panel(const PanelCreateInfo& panel_create_info) {
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

    // Load panel layout from disk, then create a panel from it
    Panel& load_panel(const char* path, const glm::vec2 top_left) {
        auto layout = toml::parse_file(path);

        // Parse metadata
        auto panel_meta = layout["panel_meta"];
        assert(panel_meta);

        PanelCreateInfo create_info = {.top_left = top_left};
        const auto name             = panel_meta["name"].value<std::string_view>().value_or("untitled panel");
        const auto default_size     = panel_meta["default_size"].as_array();
        const auto min_size         = panel_meta["min_size"].as_array();
        const auto max_size         = panel_meta["max_size"].as_array();
        const auto bg_color         = panel_meta["bg_color"].as_array();
        create_info.name            = name;
        if (min_size && min_size->is_array() && min_size->size() >= 2) {
            create_info.min_size.x = (*min_size)[0].value_or<float>(128.0f);
            create_info.min_size.y = (*min_size)[1].value_or<float>(128.0f);
        }
        if (max_size && max_size->is_array() && max_size->size() >= 2) {
            create_info.max_size.x = (*max_size)[0].value_or<float>(99999.0f);
            create_info.max_size.y = (*max_size)[1].value_or<float>(99999.0f);
        }
        if (bg_color && bg_color->is_array() && bg_color->size() >= 3) {
            create_info.bg_color.r = (*bg_color)[0].value_or<float>(0.1f);
            create_info.bg_color.g = (*bg_color)[1].value_or<float>(0.1f);
            create_info.bg_color.b = (*bg_color)[2].value_or<float>(0.2f);
            if (bg_color->size() >= 4) {
                create_info.bg_color.a = (*bg_color)[3].value_or<float>(1.0f);
            }
        }
        if (default_size && default_size->is_array() && default_size->size() >= 2) {
            create_info.size.x = (*default_size)[0].value_or<float>(128.0f);
            create_info.size.y = (*default_size)[1].value_or<float>(128.0f);
        } else {
            create_info.size = create_info.min_size;
        }

        // Parse UI elements
        if (auto elements = layout["elements"].as_table()) {
            for (auto& [name, node]: *elements) {
                if (auto node_tbl = node.as_table()) {
                    // Skip the element if it has no type node. Every node should have this type
                    if (node_tbl->find("type") == node_tbl->end()) {
                        LOG(Warning, "Panel layout from file \"%s\" has element \"%.*s\" with no type! Skipping element", path,
                            name.length(), name.data());
                        continue;
                    }

                    auto type = (*node_tbl)["type"].as_string();
                    LOG(Info, "Element \"%.*s\" is of type \"%s\"", name.length(), name.data(), (*type)->c_str());
                }
            }

            return new_panel(create_info);
        }
    }

    void panel_input() {
        // If a panel is being dragged or resized, that's the one we should focus on. Otherwise, update whatever panels are
        // below the mouse
        size_t panel_to_focus_on = -1;
        bool do_mouse_interact   = true;

        for (const auto& index: panel_order) {
            const auto& panel = panel_pool[index];
            if (panel.being_dragged || panel.being_resized) {
                panel_to_focus_on = index;
                panel_pool[index].update(Gfx::get_delta_time(), do_mouse_interact);
            }
        }

        if (panel_to_focus_on == -1) {
            for (const auto& index: panel_order) {
                const auto& panel   = panel_pool[index];
                Hitbox panel_hitbox = {
                    .top_left     = panel.top_left - glm::vec2(resize_sensitivity),
                    .bottom_right = panel.top_left + panel.size + glm::vec2(resize_sensitivity)};
                if (panel_hitbox.intersects(Input::mouse_position_pixels())) {
                    panel_pool[index].update(Gfx::get_delta_time(), do_mouse_interact);
                    do_mouse_interact = false;
                    if (Input::mouse_button_pressed(Input::MouseButton::Left)) {
                        panel_to_focus_on = index;
                        break;
                    }
                }
            }
        }

        if (panel_to_focus_on != -1 && prev_panel_to_focus_on != panel_to_focus_on) {
            panel_order_scratch.clear();
            panel_order_scratch.push_back(panel_to_focus_on);
            for (const auto& value: panel_order) {
                if (value == panel_to_focus_on) continue;
                panel_order_scratch.push_back(value);
            }
            panel_order            = panel_order_scratch;
            prev_panel_to_focus_on = panel_to_focus_on;
            panels_dirty           = true;
        }
    }

    void panel_render() {
        for (size_t i = panel_order.size(); i-- > 0;) {
            panel_pool[panel_order[i]].render_window();
        }
    }
} // namespace UI
