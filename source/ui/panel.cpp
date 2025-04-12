#include "panel.hpp"
#include "components.hpp"
#include "../graphics/renderer.hpp"
namespace UI {

    void Panel::update(float delta_time) {
        constexpr float window_bar_height = 40.0f;
        constexpr float snap_sensitivity  = 32.0f;

        // Window dragging
        if (Input::mouse_button_pressed(Input::MouseButton::Left) && Input::mouse_position_pixels().x > this->top_left.x &&
            Input::mouse_position_pixels().x < this->top_left.x + this->size.x &&
            Input::mouse_position_pixels().y > this->top_left.y &&
            Input::mouse_position_pixels().y < this->top_left.y + window_bar_height) {
            this->being_dragged = true;
        }

        if (this->being_dragged && Input::mouse_button_released(Input::MouseButton::Left)) {
            this->being_dragged = false;
        }

        if (this->being_dragged) {
            const glm::vec2 mouse_movement = Input::mouse_movement_pixels();
            const glm::vec2 parent_size    = Gfx::get_window_size(); // todo: nested panels should reference the parent
            this->top_left += mouse_movement;

            // Snap to sides
            if (mouse_movement.x < 0.0f && this->top_left.x < snap_sensitivity) {
                this->top_left.x = 0.0f;
            }
            if (mouse_movement.x > 0.0f && (parent_size.x - this->top_left.x - this->size.x) < snap_sensitivity) {
                this->top_left.x = parent_size.x - this->size.x;
            }
            if (mouse_movement.y < 0.0f && this->top_left.y < snap_sensitivity) {
                this->top_left.y = 0.0f;
            }
            if (mouse_movement.y > 0.0f && (parent_size.y - this->top_left.y - this->size.y) < snap_sensitivity) {
                this->top_left.y = parent_size.y - this->size.y;
            }
        }

        // Update panel size
        scene.update_extents(this->top_left, this->size, window_bar_height);

        // Rendering
        Gfx::draw_text_pixels( // Panel name
            name,
            (Gfx::TextDrawParams){
                .transform = {.position = glm::vec3(this->top_left + glm::vec2(4.0f, 6.0f), 0.0f), .scale = {2.0f, 2.0f, 1.0f}},
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
