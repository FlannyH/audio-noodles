#include "panel.hpp"
#include "components.hpp"
#include "../graphics/renderer.hpp"
namespace UI {

    void Panel::update(float delta_time) {
        constexpr float window_bar_height  = 40.0f;
        constexpr float snap_sensitivity   = 32.0f;
        constexpr float unmax_sensitivity  = 32.0f;
        constexpr float resize_sensitivity = 16.0f;
        constexpr float double_click_time  = 0.3f;

        // Window dragging
        const glm::vec2 mouse_pos      = Input::mouse_position_pixels();
        const glm::vec2 mouse_movement = Input::mouse_movement_pixels();
        const bool is_mouse_inside_title_bar =
            mouse_pos.x > this->top_left.x && mouse_pos.x < this->top_left.x + this->size.x && mouse_pos.y > this->top_left.y &&
            mouse_pos.y < this->top_left.y + window_bar_height;

        if (is_mouse_inside_title_bar) {
            Gfx::set_cursor_mode(Gfx::CursorMode::Hand);
        }

        if (Input::mouse_button_pressed(Input::MouseButton::Left) && is_mouse_inside_title_bar) {
            this->being_dragged        = true;
            this->begin_drag_mouse_pos = mouse_pos;
        }

        if (this->being_dragged && Input::mouse_button_released(Input::MouseButton::Left)) {
            this->being_dragged = false;
        }

        if (this->being_dragged) {
            const glm::vec2 parent_size = Gfx::get_window_size(); // todo: nested panels should reference the parent
            this->top_left += mouse_movement;

            // Unmaximize
            if (this->maximized && glm::distance(mouse_pos, this->begin_drag_mouse_pos) > unmax_sensitivity) {
                this->top_left  = mouse_pos - glm::vec2(this->pre_max_size.x / 2.0f, window_bar_height / 2.0f);
                this->size      = this->pre_max_size;
                this->maximized = false;
            }

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

        // Double click title bar to maximize and unmaximize
        if (is_mouse_inside_title_bar && Input::mouse_button_pressed(Input::MouseButton::Left)) {
            if (this->double_click_timer > 0.0f) {
                if (this->maximized) {
                    this->top_left  = this->pre_max_top_left;
                    this->size      = this->pre_max_size;
                    this->maximized = false;
                } else {
                    this->pre_max_top_left = this->top_left;
                    this->pre_max_size     = this->size;
                    this->maximized        = true;
                }
            }
            this->double_click_timer = double_click_time;
        }
        double_click_timer -= delta_time;

        // Update maximized locations every frame (the user might have resized the parent)
        if (this->maximized) {
            this->top_left = glm::vec2(0.0f, 0.0f);
            this->size     = Gfx::get_window_size();
        }

        // Resizing
        int new_resize_flags   = 0;
        constexpr int resize_l = 1;
        constexpr int resize_r = 2;
        constexpr int resize_t = 4;
        constexpr int resize_b = 8;

        if (abs(mouse_pos.x - (this->top_left.x + this->size.x)) < resize_sensitivity) new_resize_flags |= resize_l;
        if (abs(mouse_pos.x - this->top_left.x) < resize_sensitivity) new_resize_flags |= resize_r;
        if (abs(mouse_pos.y - (this->top_left.y + this->size.y)) < resize_sensitivity) new_resize_flags |= resize_b;
        if (abs(mouse_pos.y - this->top_left.y) < resize_sensitivity) new_resize_flags |= resize_t;

        if (is_mouse_inside_title_bar == false) { // Only show resize if not focused on the title bar
            if (new_resize_flags == (resize_l) || new_resize_flags == (resize_r))
                Gfx::set_cursor_mode(Gfx::CursorMode::ResizeEW);
            if (new_resize_flags == (resize_t) || new_resize_flags == (resize_b))
                Gfx::set_cursor_mode(Gfx::CursorMode::ResizeNS);
            if (new_resize_flags == (resize_t | resize_l) || new_resize_flags == (resize_b | resize_r))
                Gfx::set_cursor_mode(Gfx::CursorMode::ResizeNESW);
            if (new_resize_flags == (resize_t | resize_r) || new_resize_flags == (resize_b | resize_l))
                Gfx::set_cursor_mode(Gfx::CursorMode::ResizeNWSE);
        }

        if (is_mouse_inside_title_bar == false && Input::mouse_button_pressed(Input::MouseButton::Left)) {
            this->resize_flags  = new_resize_flags;
            this->being_resized = true;
        }

        if (this->being_resized) {
            if (this->resize_flags & resize_r) {
                this->top_left.x += mouse_movement.x;
                this->size.x -= mouse_movement.x;
            }
            if (this->resize_flags & resize_l) {
                this->size.x += mouse_movement.x;
            }
            if (this->resize_flags & resize_b) {
                this->size.y += mouse_movement.y;
            }
            if (this->resize_flags & resize_t) {
                this->top_left.y += mouse_movement.y;
                this->size.y -= mouse_movement.y;
            }
            if (Input::mouse_button_released(Input::MouseButton::Left)) this->being_resized = false;
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
            (Gfx::DrawParams){.anchor_point = Gfx::AnchorPoint::TopLeft, .shape_outline_width = 2.0f});
        Gfx::draw_rectangle_2d_pixels( // Content background color
            this->top_left + glm::vec2(0, window_bar_height), this->top_left + this->size,
            {.color = this->bg_color, .anchor_point = Gfx::AnchorPoint::TopLeft});
        Gfx::draw_rectangle_2d_pixels( // Content border
            this->top_left + glm::vec2(0, window_bar_height), this->top_left + this->size,
            (Gfx::DrawParams){.anchor_point = Gfx::AnchorPoint::TopLeft, .shape_outline_width = 2.0f});
        UI::update_entities(this->scene, delta_time, window_bar_height); // Content
    }
} // namespace UI
