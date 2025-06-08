#pragma once
#include "panel.hpp"

namespace UI {
    struct PanelCreateInfo {
        std::string name = "Yippee panels have names now :3";
        glm::vec2 top_left;
        glm::vec2 size;
        glm::vec2 min_size = {128.0f, 128.0f};
        glm::vec2 max_size = {99999.0f, 99999.0f};
        glm::vec4 bg_color = glm::vec4(0.1f, 0.1f, 0.2f, 1.0f);
        bool maximized     = false;
    };
    std::vector<Panel*>& get_panels();
    Panel& new_panel(PanelCreateInfo&& panel_create_info);
    void panel_input();
    void panel_render();
} // namespace UI
