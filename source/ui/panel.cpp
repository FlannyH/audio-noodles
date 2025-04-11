#include "panel.hpp"
#include "components.hpp"
#include "../graphics/renderer.hpp"

namespace UI {
    void Panel::update(float delta_time) {
        scene.update_extents(this->top_left, this->size);
        UI::update_entities(this->scene, delta_time);
        Gfx::draw_rectangle_2d_pixels(
            this->top_left, this->top_left + this->size, (Gfx::DrawParams){.anchor_point = Gfx::AnchorPoint::TopLeft, .rectangle_outline_width = 2.0f});
    }
} // namespace UI
