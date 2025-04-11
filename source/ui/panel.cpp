#include "panel.hpp"
#include "components.hpp"
#include "../graphics/renderer.hpp"

namespace UI {
    void Panel::update(float delta_time) {
        UI::update_entities(this->scene, delta_time);
        Gfx::draw_rectangle_2d_pixels(
            this->top_left, this->top_left + this->size, (Gfx::DrawParams){.anchor_point = Gfx::AnchorPoint::TopLeft});
    }
} // namespace UI
