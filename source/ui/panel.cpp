#include "panel.hpp"
#include "components.hpp"
#include "panel_manager.hpp"
#include "../graphics/renderer.hpp"
namespace UI {
    void Panel::update(float delta_time, bool do_mouse_interact) {
        const bool should_snap = Input::key_held(Input::Key::LeftShift) || Input::key_held(Input::Key::RightShift);

        // Window dragging
        const glm::vec2 mouse_pos      = Input::mouse_position_pixels();
        const glm::vec2 mouse_movement = Input::mouse_movement_pixels();

        const Hitbox title_bar = {
            .top_left = this->top_left, .bottom_right = this->top_left + glm::vec2(this->size.x, window_bar_height)};
        const Hitbox panel_padded = {
            .top_left     = this->top_left - glm::vec2(resize_sensitivity),
            .bottom_right = this->top_left + this->size + glm::vec2(resize_sensitivity)};
        const bool is_mouse_inside_title_bar = title_bar.intersects(mouse_pos);
        const bool is_mouse_inside_panel     = panel_padded.intersects(mouse_pos);

        if (do_mouse_interact && is_mouse_inside_title_bar) {
            Gfx::set_cursor_mode(Gfx::CursorMode::Hand);
        }

        if (Input::mouse_button_pressed(Input::MouseButton::Left) && is_mouse_inside_title_bar) {
            this->being_dragged        = true;
            this->begin_drag_mouse_pos = mouse_pos;
            this->begin_drag_top_left  = this->top_left;
        }

        if (this->being_dragged && Input::mouse_button_released(Input::MouseButton::Left)) {
            this->being_dragged = false;
        }

        if (this->being_dragged) {
            const glm::vec2 parent_size = Gfx::get_viewport_size(); // todo: nested panels should reference the parent
            this->top_left              = (this->begin_drag_top_left - this->begin_drag_mouse_pos) + mouse_pos;

            // Unmaximize
            if (this->maximized && glm::distance(mouse_pos, this->begin_drag_mouse_pos) > unmax_distance) {
                this->top_left  = mouse_pos - glm::vec2(this->pre_max_size.x / 2.0f, window_bar_height / 2.0f);
                this->size      = this->pre_max_size;
                this->maximized = false;
            }

            // Snap to sides when holding shift
            if (should_snap) {
                if ((mouse_movement.x < 0.0f && this->top_left.x < snap_sensitivity) || this->top_left.x < 0.0f) {
                    this->top_left.x = 0.0f;
                }
                if ((mouse_movement.x > 0.0f && (parent_size.x - this->top_left.x - this->size.x) < snap_sensitivity) ||
                    (this->top_left.x > (parent_size.x - this->size.x))) {
                    this->top_left.x = parent_size.x - this->size.x;
                }
                if ((mouse_movement.y < 0.0f && this->top_left.y < snap_sensitivity) || this->top_left.y < 0.0f) {
                    this->top_left.y = 0.0f;
                }
                if ((mouse_movement.y > 0.0f && (parent_size.y - this->top_left.y - this->size.y) < snap_sensitivity) ||
                    (this->top_left.y > (parent_size.y - this->size.y))) {
                    this->top_left.y = parent_size.y - this->size.y;
                }

                // Snap to other panels
                for (const auto& panel: UI::get_panels()) {
                    if (abs(this->top_left.x - (panel->top_left.x + panel->size.x)) < snap_sensitivity) {
                        this->top_left.x = (panel->top_left.x + panel->size.x);
                    }
                    if (abs((this->top_left.x + this->size.x) - panel->top_left.x) < snap_sensitivity) {
                        this->top_left.x = (panel->top_left.x - this->size.x);
                    }
                    if (abs(this->top_left.y - (panel->top_left.y + panel->size.y)) < snap_sensitivity) {
                        this->top_left.y = (panel->top_left.y + panel->size.y);
                    }
                    if (abs((this->top_left.y + this->size.y) - panel->top_left.y) < snap_sensitivity) {
                        this->top_left.y = (panel->top_left.y - this->size.y);
                    }
                }
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
                    this->being_dragged    = false;
                }
            }
            this->double_click_timer = double_click_time;
        }
        double_click_timer -= delta_time;

        // Update maximized locations every frame (the user might have resized the parent)
        if (this->maximized) {
            this->top_left = glm::vec2(0.0f, 0.0f);
            this->size.x   = std::clamp(Gfx::get_viewport_size().x, min_size.x, max_size.x);
            this->size.y   = std::clamp(Gfx::get_viewport_size().y, min_size.y, max_size.y);
        }

        // Resizing
        int new_resize_flags   = 0;
        constexpr int resize_l = 1;
        constexpr int resize_r = 2;
        constexpr int resize_t = 4;
        constexpr int resize_b = 8;

        if (abs(mouse_pos.x - (this->top_left.x + this->size.x)) < resize_sensitivity) new_resize_flags |= resize_r;
        if (abs(mouse_pos.x - this->top_left.x) < resize_sensitivity) new_resize_flags |= resize_l;
        if (abs(mouse_pos.y - (this->top_left.y + this->size.y)) < resize_sensitivity) new_resize_flags |= resize_b;
        if (abs(mouse_pos.y - this->top_left.y) < resize_sensitivity) new_resize_flags |= resize_t;

        if (do_mouse_interact && is_mouse_inside_title_bar == false &&
            is_mouse_inside_panel) { // Only show resize if not focused on the title bar
            if (new_resize_flags == (resize_l) || new_resize_flags == (resize_r))
                Gfx::set_cursor_mode(Gfx::CursorMode::ResizeEW);
            if (new_resize_flags == (resize_t) || new_resize_flags == (resize_b))
                Gfx::set_cursor_mode(Gfx::CursorMode::ResizeNS);
            if (new_resize_flags == (resize_t | resize_r) || new_resize_flags == (resize_b | resize_l))
                Gfx::set_cursor_mode(Gfx::CursorMode::ResizeNESW);
            if (new_resize_flags == (resize_t | resize_l) || new_resize_flags == (resize_b | resize_r))
                Gfx::set_cursor_mode(Gfx::CursorMode::ResizeNWSE);
        }

        if (is_mouse_inside_title_bar == false && is_mouse_inside_panel &&
            Input::mouse_button_pressed(Input::MouseButton::Left)) {
            this->resize_flags  = new_resize_flags;
            this->being_resized = true;
        }

        if (this->being_resized) {
            if (this->resize_flags & resize_r) this->set_right(mouse_pos.x);
            if (this->resize_flags & resize_l) this->set_left(mouse_pos.x);
            if (this->resize_flags & resize_b) this->set_bottom(mouse_pos.y);
            if (this->resize_flags & resize_t) this->set_top(mouse_pos.y);
            if (should_snap) {
                if (this->resize_flags & resize_r && abs(mouse_pos.x - (Gfx::get_viewport_size().x)) < snap_sensitivity) {
                    this->set_right(Gfx::get_viewport_size().x);
                }
                if (this->resize_flags & resize_l && abs(mouse_pos.x - (0.0f)) < snap_sensitivity) {
                    this->set_left(0.0f);
                }
                if (this->resize_flags & resize_b && abs(mouse_pos.y - (Gfx::get_viewport_size().y)) < snap_sensitivity) {
                    this->set_bottom(Gfx::get_viewport_size().y);
                }
                if (this->resize_flags & resize_t && abs(mouse_pos.y - (0.0f)) < snap_sensitivity) {
                    this->set_top(0.0f);
                }

                for (const auto& panel: UI::get_panels()) {
                    if (this->resize_flags & resize_r && abs(mouse_pos.x - (panel->top_left.x)) < snap_sensitivity) {
                        this->set_right(panel->top_left.x);
                    }
                    if (this->resize_flags & resize_l &&
                        abs(mouse_pos.x - (panel->top_left.x + panel->size.x)) < snap_sensitivity) {
                        this->set_left(panel->top_left.x + panel->size.x);
                    }
                    if (this->resize_flags & resize_b && abs(mouse_pos.y - (panel->top_left.y)) < snap_sensitivity) {
                        this->set_bottom(panel->top_left.y);
                    }
                    if (this->resize_flags & resize_t &&
                        abs(mouse_pos.y - (panel->top_left.y + panel->size.y)) < snap_sensitivity) {
                        this->set_top(panel->top_left.y + panel->size.y);
                    }
                }
            }
            if (Input::mouse_button_released(Input::MouseButton::Left)) this->being_resized = false;
        }

        // Update panel size
        glm::ivec2 content_size = {(int)this->size.x - 2, (int)(this->size.y - window_bar_height - 2)};
        if (this->size_prev != this->size) {
            if (this->content_render_target.is_valid() == false) {
                this->content_render_target = Gfx::create_texture_from_data({
                    .format         = Gfx::PixelFormat::RGBA_8,
                    .type           = Gfx::TextureType::Single2D,
                    .width          = (size_t)content_size.x,
                    .height         = (size_t)content_size.y,
                    .depth          = 1,
                    .is_framebuffer = true,
                    .data           = nullptr,
                });
            } else {
                Gfx::resize_texture(this->content_render_target, glm::ivec3(content_size, 0));
            }
            this->size_prev = this->size;
        }
        scene.update_extents(this->top_left + glm::vec2(1, window_bar_height + 1), content_size);
        UI::update_entities_input(this->scene, delta_time, do_mouse_interact);
    }

    void Panel::render_window() {
        // Render content to separate render target
        const glm::ivec2 content_size = {(int)this->size.x - 2, (int)(this->size.y - window_bar_height - 2)};
        Gfx::set_render_target(this->content_render_target);
        Gfx::set_viewport({0, 0}, content_size);
        Gfx::push_clip_rect({0, 0}, content_size);
        Gfx::clear_framebuffer({});
        Gfx::draw_rectangle_2d(
            {-1, -1}, {1, 1},
            {
                .color               = this->bg_color,
                .depth               = 1.0f,
                .shape_outline_width = 0.0f,
            });
        UI::update_entities_render(this->scene);
        Gfx::pop_clip_rect();

        Gfx::set_render_target();
        Gfx::set_viewport({0, 0}, Gfx::get_window_size());
        Gfx::push_clip_rect(this->top_left, this->size);
        Gfx::draw_rectangle_2d_pixels( // Title bar background
            this->top_left, this->top_left + glm::vec2(this->size.x, window_bar_height + 2),
            Gfx::DrawParams{
                .color = Colors::DARK_BLUE, .anchor_point = Gfx::AnchorPoint::TopLeft, .shape_outline_width = 0.0f});
        Gfx::draw_text_pixels( // Panel name
            this->title,
            Gfx::TextDrawParams{
                .transform = {.position = glm::vec3(this->top_left + glm::vec2(4.0f, 6.0f), 0.0f), .scale = {2.0f, 2.0f, 1.0f}},
                .position_anchor = Gfx::AnchorPoint::TopLeft,
                .color           = Colors::WHITE,
            });
        Gfx::draw_rectangle_2d_pixels( // Title bar border
            this->top_left, this->top_left + glm::vec2(this->size.x, window_bar_height + 2),
            Gfx::DrawParams{.anchor_point = Gfx::AnchorPoint::TopLeft, .shape_outline_width = 2.0f});
        Gfx::draw_rectangle_2d_pixels( // Content border
            this->top_left + glm::vec2(0, window_bar_height), this->top_left + this->size,
            Gfx::DrawParams{.anchor_point = Gfx::AnchorPoint::TopLeft, .shape_outline_width = 2.0f});
        Gfx::blit_pixels( // Content
            this->content_render_target, Gfx::ResourceID::invalid(), this->size - glm::vec2(2, window_bar_height + 2),
            {this->top_left + glm::vec2(1, 1 + window_bar_height)}, {0, 0});
        Gfx::pop_clip_rect();
    }

    void Panel::set_top(float top) {
        float new_size = this->size.y + this->top_left.y - top;
        if (new_size > max_size.y) new_size = max_size.y;
        if (new_size < min_size.y) new_size = min_size.y;
        float difference_in_size = this->size.y - new_size;
        this->size.y -= difference_in_size;
        this->top_left.y += difference_in_size;
    }

    void Panel::set_bottom(float bottom) {
        this->size.y = bottom - this->top_left.y;
        if (this->size.y > this->max_size.y) this->size.y = this->max_size.y;
        if (this->size.y < this->min_size.y) this->size.y = this->min_size.y;
    }

    void Panel::set_left(float left) {
        float new_size = this->size.x + this->top_left.x - left;
        if (new_size > max_size.x) new_size = max_size.x;
        if (new_size < min_size.x) new_size = min_size.x;
        float difference_in_size = this->size.x - new_size;
        this->size.x -= difference_in_size;
        this->top_left.x += difference_in_size;
    }

    void Panel::set_right(float right) {
        this->size.x = right - this->top_left.x;
        if (this->size.x > this->max_size.x) this->size.x = this->max_size.x;
        if (this->size.x < this->min_size.x) this->size.x = this->min_size.x;
    }

} // namespace UI
