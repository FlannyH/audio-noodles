#include "panel.hpp"
#include "components.hpp"
#include "../graphics/renderer.hpp"
namespace UI {

    void Panel::update(float delta_time) {
        constexpr float window_bar_height = 40.0f;
        scene.update_extents(this->top_left, this->size, window_bar_height);
        Gfx::draw_text_pixels(
            name, (Gfx::TextDrawParams){
                      .transform       = {.position = glm::vec3(this->top_left + glm::vec2(4.0f, 6.0f), 0.0f), .scale = {2.0f, 2.0f, 1.0f}},
                      .position_anchor = Gfx::AnchorPoint::TopLeft,
                      .color           = Colors::WHITE,
                  });
        Gfx::draw_rectangle_2d_pixels( // Title bar border
            this->top_left, this->top_left + glm::vec2(this->size.x, window_bar_height + 2),
            (Gfx::DrawParams){.anchor_point = Gfx::AnchorPoint::TopLeft, .rectangle_outline_width = 2.0f});
        Gfx::draw_rectangle_2d_pixels( // Content background color
            this->top_left + glm::vec2(0, window_bar_height), this->top_left + this->size,
            {.color = this->bg_color, .anchor_point = Gfx::AnchorPoint::TopLeft});
        Gfx::draw_rectangle_2d_pixels( // Content border
            this->top_left + glm::vec2(0, window_bar_height), this->top_left + this->size,
            (Gfx::DrawParams){.anchor_point = Gfx::AnchorPoint::TopLeft, .rectangle_outline_width = 2.0f});
        UI::update_entities(this->scene, delta_time, window_bar_height);
    }
} // namespace UI
