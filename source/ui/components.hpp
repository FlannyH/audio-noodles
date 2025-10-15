#pragma once
#include <cmath>
#include <cstring>
#include <utility>
#include <algorithm>
#include <functional>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

#include "scene.hpp"
// #include "value_system.hpp"
#include "../graphics/renderer.hpp"

namespace UI {
    template <typename T> T sign(T v) { return (v > 0) ? +1 : -1; }

    struct Transform {
        Transform() {}
        Transform(
            const glm::vec2 tl, const glm::vec2 br, const float dpth = 0.5f,
            const Gfx::AnchorPoint anch = Gfx::AnchorPoint::TopLeft) {
            const float x_min = std::min(tl.x, br.x);
            const float x_max = std::max(tl.x, br.x);
            const float y_min = std::min(tl.y, br.y);
            const float y_max = std::max(tl.y, br.y);
            top_left          = {x_min, y_min};
            bottom_right      = {x_max, y_max};
            anchor            = anch;
            depth             = dpth;
        }
        glm::vec2 top_left{}, bottom_right{};
        float depth{};
        Gfx::AnchorPoint anchor{};
    };

    // todo: naming conventions
    enum class ClickState { idle = 0, hover, click };

    struct Clickable {};

    struct MouseInteract {
        ClickState state = ClickState::idle;
    };

    struct SpriteRender {};

    struct Sprite {
        Sprite(const std::string& path, const Gfx::TextureScaleMode type = Gfx::TextureScaleMode::Stretch) {
            tex_path = path;
            tex_type = type;
        }
        Sprite(const Sprite& other) {
            tex_path = std::string(other.tex_path);
            tex_type = other.tex_type;
        }
        std::string tex_path;
        Gfx::TextureScaleMode tex_type = Gfx::TextureScaleMode::Stretch;
    };

    struct Sprites {
        Sprites(std::initializer_list<Sprite> init_list) {
            // Insert the init_list's values
            sprites.insert(sprites.end(), init_list.begin(), init_list.end());
        }
        std::vector<Sprite> sprites;
    };

    // todo: add a version that has an editable textbox (for easy searching)
    struct Text {
        Text(
            const std::wstring& string = std::wstring(), const glm::vec2 scl = {2, 2}, const glm::vec4 col = {1, 1, 1, 1},
            const Gfx::AnchorPoint ui_anchr  = Gfx::AnchorPoint::TopLeft,
            const Gfx::AnchorPoint txt_anchr = Gfx::AnchorPoint::TopLeft) {
            text_length = string.size() + 1;
            text        = new wchar_t[text_length];
            memcpy(text, string.data(), (string.size() + 1) * sizeof(string[0]));
            ui_anchor   = ui_anchr;
            text_anchor = txt_anchr;
            color       = col;
            scale       = scl;
        }

        ~Text() { delete[] text; }

        Text(const Text& other) {
            // Copy values
            const auto tmp = std::wstring(other.text);
            text_length    = tmp.size() + 1;
            text           = new wchar_t[text_length];
            memcpy(text, tmp.data(), text_length * 2);
            ui_anchor   = other.ui_anchor;
            text_anchor = other.text_anchor;
            color       = other.color;
            scale       = other.scale;
        }

        wchar_t* text;
        size_t text_length;
        Gfx::AnchorPoint ui_anchor;
        Gfx::AnchorPoint text_anchor;
        glm::vec4 color{};
        glm::vec2 scale{};
        glm::vec2 margins = {8, 8};
    };

    struct NumberRange {
        // Bounds - the value is stored inside a Value component
        double min;
        double max;
        double step;
        double step_fine;
        double default_value           = 0.0;
        uint32_t visual_decimal_places = 2;
    };

    struct Draggable {
        bool is_horizontal = false;
    };

    struct Hitbox {
        // The minimum and maximum x and y coordinates of the hitbox, in pixels relative to the transform of the component.
        // Note that the y-coordinate of the top of the hitbox is 0 and it increases as it goes down.
        glm::vec2 top_left{}, bottom_right{};
        bool intersects(const glm::vec2 pos) const {
            return (pos.x >= top_left.x && pos.x <= bottom_right.x && pos.y >= top_left.y && pos.y <= bottom_right.y);
        }
    };

    struct MultiHitbox {
        Hitbox hitboxes[16]{};
        ClickState click_states[16]{};
        size_t n_hitboxes;
    };

    struct RadioButton {
        std::vector<std::wstring> options;
        size_t current_selected_index = 0;
    };

    // todo: add a version that has an editable textbox (for easy searching)
    struct Combobox {
        std::vector<std::wstring> list_items;
        float button_height           = 60.0f;
        float list_height             = 360.0f;
        float item_height             = 60.0f;
        float target_scroll_position  = 0.0f;
        float current_scroll_position = 0.0f;
        bool is_list_open             = false;
        int current_selected_index    = 0;
    };

    // todo: add checkbox component
    // todo: add scrollable list component
    // todo: add ui component grid component
    // todo: add transform options so you can easily create ui without hardcoding
    struct Scrollable {}; // This is a tag without data
    struct NumberBox {};  // This is a tag without data
    struct Button {};     // This is a tag without data
    struct WheelKnob {};  // This is a tag without data

    struct Slider {
        float curr_bar_length;
    };
    struct Box {
        glm::vec4 color_inner = {0.25f, 0.25f, 0.25f, 1.0f};
        glm::vec4 color_outer = {1.0f, 1.0f, 1.0f, 1.0f};
        float thickness       = 2.0f;
    };
    struct Function {
        Function(std::function<void()> click) { on_click = std::move(click); }
        std::function<void()> on_click;
    };

    inline EntityID create_button(
        Scene& scene, const UI::Transform& transform, std::function<void()> func,
        const Text& text = {L"", {2, 2}, {1, 1, 1, 1}, Gfx::AnchorPoint::Center, Gfx::AnchorPoint::Center}) {
        const EntityID entity = scene.new_entity();
        scene.add_component<UI::Transform>(entity, transform);
        scene.add_component<Function>(entity, {std::move(func)});
        scene.add_component<Clickable>(entity);
        scene.add_component<MouseInteract>(entity);
        // scene.add_component<Sprites>(entity, { {"button.png", TextureScaleMode::slice} });
        // scene.add_component<SpriteRender>(entity);
        scene.add_component<Text>(entity, text);
        scene.add_component<Button>(entity);
        scene.add_component<Box>(entity, {{0.7f, 0.7f, 0.7f, 0.7f}, {1, 1, 1, 1}, 2.0f});
        return entity;
    }

    inline EntityID
    create_text(Scene& scene, const std::string& name, const UI::Transform& transform, const Text& text, bool has_box = false) {
        // Create entity and add components
        const EntityID entity = scene.new_entity();
        scene.add_component<UI::Transform>(entity, transform);
        scene.add_component<Text>(entity, text);
        scene.add_component<Value>(entity, {name, VarType::wstring, scene.value_pool});
        if (has_box) scene.add_component<Box>(entity);

        // Bind the text string to the variable name
        wchar_t* text_to_put = new wchar_t[text.text_length + 1];
        memcpy(text_to_put, text.text, text.text_length * 2);
        scene.value_pool.set_ptr(name, text_to_put);

        return entity;
    }

    inline EntityID create_numberbox(
        Scene& scene, const std::string& name, const UI::Transform& transform,
        const NumberRange& range = {0.0, 100.0, 1.0, 0.0, 0},
        const Text& text         = {
            L"",
            {2, 2},
            {1, 1, 1, 1},
            Gfx::AnchorPoint::Center,
            Gfx::AnchorPoint::Center,
        }) {
        const EntityID entity = scene.new_entity();
        scene.add_component<UI::Transform>(entity, transform);
        scene.add_component<Value>(entity, {name, VarType::float64, scene.value_pool});
        scene.add_component<NumberRange>(entity, range);
        scene.add_component<Draggable>(entity);
        scene.add_component<Scrollable>(entity);
        scene.add_component<MouseInteract>(entity);
        scene.add_component<Box>(entity);
        // scene.add_component<Sprites>(entity, { {"numberbox.png", TextureScaleMode::slice} });
        // scene.add_component<SpriteRender>(entity);
        scene.add_component<Text>(entity, text);
        scene.get_component<Value>(entity)->set<double>(range.default_value);
        return entity;
    }

    inline EntityID create_wheelknob(
        Scene& scene, const std::string& name, const UI::Transform& transform,
        const NumberRange& range = {0.0, 100.0, 1.0, 0.0, 0},
        const Text& text         = {
            L"",
            {2, 2},
            {1, 0, 1, 1},
            Gfx::AnchorPoint::Bottom,
            Gfx::AnchorPoint::Center,
        }) {
        const EntityID entity = scene.new_entity();
        scene.add_component<UI::Transform>(entity, transform);
        scene.add_component<Value>(entity, {name, VarType::float64, scene.value_pool});
        scene.add_component<NumberRange>(entity, range);
        scene.add_component<Draggable>(entity);
        scene.add_component<Scrollable>(entity);
        scene.add_component<MouseInteract>(entity);
        scene.add_component<WheelKnob>(entity);
        scene.add_component<Text>(entity, text);
        scene.get_component<Value>(entity)->set<double>(range.default_value);
        return entity;
    }

    inline EntityID create_slider(
        Scene& scene, const std::string& name, const UI::Transform& transform,
        const NumberRange& range = {0.0, 100.0, 1.0, 0.0, 0}, const bool has_text = true,
        Text text = {
            L"",
            {2, 2},
            {1, 1, 1, 1},
            Gfx::AnchorPoint::Bottom,
            Gfx::AnchorPoint::Bottom,
        }) {
        const glm::vec2 scale    = transform.bottom_right - transform.top_left;
        const bool is_horizontal = (scale.x) > (scale.y);
        if (is_horizontal) {
            text.ui_anchor   = Gfx::AnchorPoint::Left;
            text.text_anchor = Gfx::AnchorPoint::Left;
        }

        const EntityID entity = scene.new_entity();
        scene.add_component<UI::Transform>(entity, transform);
        scene.add_component<Value>(entity, {name, VarType::float64, scene.value_pool});
        scene.add_component<NumberRange>(entity, range);
        scene.add_component<Draggable>(entity, {is_horizontal});
        scene.add_component<Scrollable>(entity);
        scene.add_component<MouseInteract>(entity);
        scene.add_component<Slider>(entity);
        if (has_text) {
            scene.add_component<Text>(entity, text);
        }

        scene.get_component<Value>(entity)->set<double>(range.default_value);
        return entity;
    }

    inline EntityID create_radio_button(
        Scene& scene, const std::string& name, const UI::Transform& transform, const std::vector<std::wstring>& options,
        const size_t initial_index = 0) {
        // Create hitboxes for the radio buttons
        MultiHitbox multihitbox{};
        float curr_y        = 0;
        const float delta_y = (transform.bottom_right.y - transform.top_left.y) / static_cast<float>(options.size());

        // Populate hitboxes
        for (size_t i = 0; i < options.size(); ++i) {
            // Hitboxes
            multihitbox.hitboxes[i].top_left.x     = 0;
            multihitbox.hitboxes[i].bottom_right.x = transform.bottom_right.x - transform.top_left.x;
            multihitbox.hitboxes[i].top_left.y     = curr_y;
            multihitbox.hitboxes[i].bottom_right.y = curr_y + delta_y;
            curr_y += delta_y;
        }
        multihitbox.n_hitboxes = options.size();

        // Create entity
        const EntityID entity = scene.new_entity();
        scene.add_component<UI::Transform>(entity, transform);
        scene.add_component<Value>(entity, {name, VarType::float64, scene.value_pool}); // todo: add int type
        scene.get_component<Value>(entity)->set(static_cast<double>(initial_index));
        scene.add_component<MultiHitbox>(entity, multihitbox);
        scene.add_component<RadioButton>(entity, {options, initial_index});
        scene.add_component<MouseInteract>(entity);
        return entity;
    }

    inline EntityID create_combobox(
        Scene& scene, const std::string& name, const UI::Transform& transform, const std::vector<std::wstring>& items,
        const size_t initial_index = 0, const float item_height = 40.0f, const float list_height = 420.0f) {
        // Create hitboxes
        MultiHitbox multi_hitbox{};
        multi_hitbox.hitboxes[0].top_left     = {0, 0};
        multi_hitbox.hitboxes[0].bottom_right = transform.bottom_right - transform.top_left;
        multi_hitbox.hitboxes[1].top_left     = {0, transform.bottom_right.y - transform.top_left.y};
        multi_hitbox.hitboxes[1].bottom_right = {
            transform.bottom_right.x - transform.top_left.x, transform.bottom_right.y + list_height};
        multi_hitbox.n_hitboxes = 2;

        // Create combobox component
        Combobox combobox{};
        combobox.is_list_open            = false;
        combobox.item_height             = item_height;
        combobox.list_height             = list_height;
        combobox.current_selected_index  = static_cast<int>(initial_index);
        combobox.current_scroll_position = 0.0f;
        combobox.list_items              = items;
        combobox.button_height           = transform.bottom_right.y - transform.top_left.y;

        // Create entity
        const EntityID entity = scene.new_entity();
        scene.add_component<UI::Transform>(entity, transform);
        scene.add_component<Value>(entity, {name, VarType::float64, scene.value_pool}); // todo: add int type
        scene.add_component<MouseInteract>(entity);
        scene.add_component<MultiHitbox>(entity, multi_hitbox);
        scene.add_component<Combobox>(entity, combobox);
        return entity;
    }

    inline EntityID create_box(Scene& scene, const UI::Transform& transform, const Box& box) {
        const EntityID entity = scene.new_entity();
        scene.add_component<UI::Transform>(entity, transform);
        scene.add_component<Box>(entity, box);
        return entity;
    }

    inline void add_function(Scene& scene, const EntityID entity, std::function<void()> func) {
        scene.add_component<Function>(entity, {std::move(func)});
    }

    inline void system_comp_sprite(Scene& scene) {
        for (const auto entity: scene.view<UI::Transform, Sprites, SpriteRender>()) {
            const auto* transform = scene.get_component<UI::Transform>(entity);
            const auto* sprite    = scene.get_component<Sprites>(entity);
            // If it has a clickable component, use that to render the button
            glm::vec4 color = {1, 1, 1, 1};
            if (const auto* mouse_interact = scene.get_component<MouseInteract>(entity)) {
                if (mouse_interact->state == ClickState::hover) {
                    color *= 0.9f;
                }
                if (mouse_interact->state == ClickState::click) {
                    color *= 0.7f;
                }
            }
            Gfx::draw_rectangle_2d(
                transform->top_left, transform->bottom_right,
                {
                    .color = color, .depth = transform->depth + 0.001f, .anchor_point = transform->anchor,
                    // todo: rework .texture = ... sprite->sprites[0].tex_path
                    // todo: rework .tex_scale_mode = ... sprite->sprites[0].tex_type
                });
            // renderer.draw_box_textured(
            //     *transform, sprite->sprites[0].tex_path, sprite->sprites[0].tex_type, transform->top_left,
            //     transform->bottom_right, color, transform->depth + 0.001f, transform->anchor);
        }
    }

    inline void system_comp_text(Scene& scene) {
        for (const auto entity: scene.view<UI::Transform, Text>()) {
            const auto* transform = scene.get_component<UI::Transform>(entity);
            auto* text            = scene.get_component<Text>(entity);
            auto* value           = scene.get_component<Value>(entity);
            const auto* slider    = scene.get_component<Slider>(entity);
            const auto* range     = scene.get_component<NumberRange>(entity);

            if (value) {
                if (value->type == VarType::wstring) {
                    const auto string = (value->get_as_ptr<wchar_t>());
                    text->text        = string;
                }
                if (value->type == VarType::float64) {
                    const double& val = scene.value_pool.get<double>(value->name);
                    if (text->text_length < 32) {
                        delete text->text;
                        text->text    = new wchar_t[32];
                        text->text[0] = 'A';
                        text->text[1] = '\0';
                    }

                    // If all parts of the range are a whole number, print as if it were an integer
                    swprintf(text->text, 32, L"%.2f", val);
                    if (range) {
                        wchar_t filter[] = L"%.xf";
                        filter[2]        = L'0' + static_cast<wchar_t>(range->visual_decimal_places);
                        swprintf(text->text, 32, filter, val);
                    }
                }
            }

            // Calculate position relative to top_left
            const glm::vec2 transform_top_left     = transform->top_left + text->margins;
            const glm::vec2 transform_bottom_right = transform->bottom_right - text->margins;
            const glm::vec2 transform_rect_size    = transform_bottom_right - transform_top_left;

            const glm::vec2 anchored_top_left =
                Gfx::anchor_offset_pixels(transform_top_left, transform->anchor, scene.panel_size);
            const glm::vec2 ui_anchored_top_left =
                Gfx::anchor_offset_pixels(anchored_top_left, text->ui_anchor, transform_rect_size);

            Gfx::draw_text_pixels(
                text->text,
                Gfx::TextDrawParams{
                    .transform =
                        {.position = glm::vec3(ui_anchored_top_left, transform->depth), .scale = glm::vec3(text->scale, 1.0f)},
                    .position_anchor = Gfx::AnchorPoint::TopLeft,
                    .text_anchor     = text->text_anchor,
                    .color           = text->color,
                });
#ifdef _DEBUG
            // todo
            // renderer.draw_circle_solid(*transform, top_left, {4, 4}, {1, 0, 1, 1});
#endif
        }
    }

    inline void system_comp_special_render(Scene& scene) {
        // Wheel knobs
        auto view = scene.view<UI::Transform, Value, NumberRange, WheelKnob>();
        for (const auto entity: view) {
            auto* transform = scene.get_component<UI::Transform>(entity);
            auto* value     = scene.get_component<Value>(entity);
            auto* range     = scene.get_component<NumberRange>(entity);

            // Draw the wheel
            const glm::vec2 top_left = Gfx::anchor_offset_pixels(transform->top_left, transform->anchor, scene.panel_size);
            const glm::vec2 bottom_right =
                Gfx::anchor_offset_pixels(transform->bottom_right, transform->anchor, scene.panel_size);
            const glm::vec2 center = (top_left + bottom_right) / 2.0f;
            const glm::vec2 scale  = center - top_left;
            double& val            = value->get_as_ref<double>();
            const float angle      = static_cast<float>(
                1.5 * 3.14159265359 + ((val - range->min) / (range->max - range->min) - 0.5) * (1.75 * 3.14159265359));
            const glm::vec2 line_b = center + glm::vec2(cosf(angle), sinf(angle)) * scale;

            Gfx::draw_circle_2d_pixels(
                center, scale,
                {
                    .color               = {1.0f, 1.0f, 1.0f, 1.0f},
                    .depth               = transform->depth + 0.002f,
                    .anchor_point        = Gfx::AnchorPoint::TopLeft,
                    .shape_outline_width = 0.0f,
                });
            Gfx::draw_circle_2d_pixels(
                center, scale,
                {
                    .color               = {0.0f, 0.0f, 0.0f, 1.0f},
                    .depth               = transform->depth + 0.001f,
                    .anchor_point        = Gfx::AnchorPoint::TopLeft,
                    .shape_outline_width = 2.0f,
                });
            Gfx::draw_line_2d_pixels(
                center, line_b,
                {
                    .color        = {0.0f, 0.0f, 0.0f, 1.0f},
                    .depth        = transform->depth + 0.001f,
                    .anchor_point = Gfx::AnchorPoint::TopLeft,
                    .line_width   = 3.0f,
                });
        }

        // Sliders
        for (const auto entity: scene.view<UI::Transform, Value, NumberRange, Slider>()) {
            auto* transform = scene.get_component<UI::Transform>(entity);
            auto* value     = scene.get_component<Value>(entity);
            auto* range     = scene.get_component<NumberRange>(entity);
            auto* text      = scene.get_component<Text>(entity);
            auto* draggable = scene.get_component<Draggable>(entity);
            auto* slider    = scene.get_component<Slider>(entity);

            // Draw the slider
            glm::vec2 top_left     = Gfx::anchor_offset_pixels(transform->top_left, transform->anchor, scene.panel_size);
            glm::vec2 bottom_right = Gfx::anchor_offset_pixels(transform->bottom_right, transform->anchor, scene.panel_size);

            if (text && draggable) {
                if (draggable->is_horizontal == false) {
                    bottom_right.y -= Gfx::get_font_height() * text->scale.y;
                    bottom_right.y -= 4;
                    slider->curr_bar_length = bottom_right.y - top_left.y;
                } else {
                    top_left.x += Gfx::get_font_max_width() * (range->visual_decimal_places + 3) * text->scale.x;
                    bottom_right.x -= 4;
                    slider->curr_bar_length = bottom_right.x - top_left.x;
                }
            }
            const glm::vec2 center = (top_left + bottom_right) / 2.0f;
            const glm::vec2 scale  = center - top_left;
            double& val            = value->get_as_ref<double>();
            float margin           = 8;
            if (draggable && draggable->is_horizontal) {
                const float dist_left =
                    static_cast<float>(((val - range->min) / (range->max - range->min) - 0.5)) * 2 * (scale.x - margin);
                Gfx::draw_line_2d_pixels(
                    center + glm::vec2{scale.x, 0}, center - glm::vec2{scale.x, 0},
                    {
                        .color        = Colors::WHITE,
                        .depth        = transform->depth + 0.0002f,
                        .anchor_point = Gfx::AnchorPoint::TopLeft,
                        .line_width   = 4.0f,
                    });
                Gfx::draw_line_2d_pixels(
                    center + glm::vec2{scale.x, 0}, center - glm::vec2{scale.x, 0},
                    {
                        .color        = Colors::WHITE,
                        .depth        = transform->depth + 0.0001f,
                        .anchor_point = Gfx::AnchorPoint::TopLeft,
                        .line_width   = 2.0f,
                    });
                Gfx::draw_rectangle_2d_pixels(
                    center + glm::vec2{dist_left - 10, -20}, center + glm::vec2{dist_left + 10, +20},
                    {
                        .color        = Colors::WHITE,
                        .depth        = transform->depth,
                        .anchor_point = Gfx::AnchorPoint::TopLeft,
                    });
            } else {
                const float dist_bottom =
                    static_cast<float>(((val - range->min) / (range->max - range->min) - 0.5)) * 2 * -(scale.y - margin);
                Gfx::draw_line_2d_pixels(
                    center + glm::vec2{0, scale.y}, center - glm::vec2{0, scale.y},
                    {
                        .color        = Colors::WHITE,
                        .depth        = transform->depth + 0.0002f,
                        .anchor_point = Gfx::AnchorPoint::TopLeft,
                        .line_width   = 4.0f,
                    });
                Gfx::draw_line_2d_pixels(
                    center + glm::vec2{0, scale.y}, center - glm::vec2{0, scale.y},
                    {
                        .color        = Colors::WHITE,
                        .depth        = transform->depth + 0.0001f,
                        .anchor_point = Gfx::AnchorPoint::TopLeft,
                        .line_width   = 2.0f,
                    });
                Gfx::draw_rectangle_2d_pixels(
                    center + glm::vec2{-20, dist_bottom - 10}, center + glm::vec2{+20, dist_bottom + 10},
                    {
                        .color        = Colors::WHITE,
                        .depth        = transform->depth,
                        .anchor_point = Gfx::AnchorPoint::TopLeft,
                    });
            }
        }

        // Radio buttons
        for (const auto entity: scene.view<UI::Transform, Value, RadioButton>()) {
            auto* transform    = scene.get_component<UI::Transform>(entity);
            auto* value        = scene.get_component<Value>(entity);
            auto* radio_button = scene.get_component<RadioButton>(entity);

            // Update the radio button current index
            radio_button->current_selected_index = static_cast<size_t>(value->get_as_ref<double>());

            // Get some information ready for the sake of my mental sanity in writing this code
            size_t n_options             = radio_button->options.size();
            float vertical_spacing       = (transform->bottom_right.y - transform->top_left.y) / static_cast<float>(n_options);
            float margin                 = 2.0f;
            float circle_size_max        = vertical_spacing / 2.0f - margin;
            glm::vec2 circle_base_offset = Gfx::anchor_offset_pixels(transform->top_left, transform->anchor, scene.panel_size) +
                                           glm::vec2(margin + circle_size_max);
            float outline_circle_radius  = 16;
            float selected_circle_radius = 12;
            float text_margin            = 20;

            // Display every option
            for (size_t i = 0; i < n_options; ++i) {
                // Determine a nice color based on what the mouse is doing
                glm::vec4 color          = {1, 1, 1, 1};
                const auto* multi_hitbox = scene.get_component<MultiHitbox>(entity);
                // if (multi_hitbox != nullptr && i == radio_button->current_selected_index) {
                if (multi_hitbox != nullptr) {
                    if (multi_hitbox->click_states[i] == ClickState::hover) {
                        color *= 0.9f;
                    }
                    if (multi_hitbox->click_states[i] == ClickState::click) {
                        color *= 0.7f;
                    }
                }

                // Draw the circle outline for each of them
                Gfx::draw_circle_2d_pixels(
                    circle_base_offset + glm::vec2(0, vertical_spacing * static_cast<float>(i)),
                    glm::vec2(outline_circle_radius),
                    {
                        .color               = color,
                        .depth               = transform->depth,
                        .anchor_point        = Gfx::AnchorPoint::TopLeft,
                        .shape_outline_width = 2.0f,
                    });

                // Draw the text
                Gfx::draw_text_pixels(
                    radio_button->options[i].c_str(),
                    Gfx::TextDrawParams{
                        .transform =
                            {.position = glm::vec3(
                                 circle_base_offset + glm::vec2(0, vertical_spacing * static_cast<float>(i)) +
                                     glm::vec2(outline_circle_radius + text_margin, 0),
                                 transform->depth),
                             .scale = {2.0f, 2.0f, 1.0f}},
                        .position_anchor = Gfx::AnchorPoint::TopLeft,
                        .text_anchor     = Gfx::AnchorPoint::Left,
                        .color           = color,
                    });

                // Draw selected circle
                if (i == radio_button->current_selected_index) {
                    Gfx::draw_circle_2d_pixels(
                        circle_base_offset + glm::vec2(0, vertical_spacing * static_cast<float>(i)),
                        glm::vec2(selected_circle_radius),
                        {.color = color, .depth = transform->depth, .anchor_point = Gfx::AnchorPoint::TopLeft});
                }
            }
        }

        // Combobox
        for (const auto entity: scene.view<UI::Transform, Combobox, MultiHitbox, Value>()) {
            auto* transform    = scene.get_component<UI::Transform>(entity);
            auto* combobox     = scene.get_component<Combobox>(entity);
            auto* multi_hitbox = scene.get_component<MultiHitbox>(entity);
            auto* value        = scene.get_component<Value>(entity);

            // Determine a nice color based on what the mouse is doing
            glm::vec4 top_color = {1, 1, 1, 1};

            // todo: use actual color schemes instead of hard coded magic numbers
            if (multi_hitbox->click_states[0] == ClickState::hover) {
                top_color *= 0.9f;
            }
            if (multi_hitbox->click_states[0] == ClickState::click) {
                top_color *= 0.7f;
            }

            // Render the button
            glm::vec2 top_left     = Gfx::anchor_offset_pixels(transform->top_left, transform->anchor, scene.panel_size);
            glm::vec2 bottom_right = Gfx::anchor_offset_pixels(transform->bottom_right, transform->anchor, scene.panel_size);
            glm::vec2 box_top_left = top_left;
            glm::vec2 box_bottom_right = {bottom_right.x, top_left.y + combobox->button_height};
            glm::vec2 arrow_center     = {bottom_right.x - 30.f, top_left.y + (combobox->button_height / 2) + 8};
            glm::vec2 text_offset      = {8, (combobox->button_height / 2)};
            glm::vec2 arrow_offset     = {16, -16};

            Gfx::draw_rectangle_2d_pixels(
                box_top_left, box_bottom_right,
                {.color               = top_color,
                 .depth               = transform->depth + 0.001f,
                 .anchor_point        = Gfx::AnchorPoint::TopLeft,
                 .shape_outline_width = 0.0f});
            Gfx::draw_rectangle_2d_pixels(
                box_top_left, box_bottom_right,
                {.color               = Colors::BLACK,
                 .depth               = transform->depth,
                 .anchor_point        = Gfx::AnchorPoint::TopLeft,
                 .shape_outline_width = 1.0f});

            const wchar_t* text_string = L"<no item selected>";
            if (combobox->current_selected_index != -1) {
                text_string = combobox->list_items[combobox->current_selected_index].c_str();
            }

            Gfx::draw_text_pixels(
                text_string, {
                                 .transform =
                                     {
                                         .position = glm::vec3(top_left + text_offset, transform->depth),
                                         .scale    = {2.0f, 2.0f, 1.0f},
                                     },
                                 .position_anchor = Gfx::AnchorPoint::TopLeft,
                                 .text_anchor     = Gfx::AnchorPoint::Left,
                                 .color           = Colors::BLACK,
                             });

            Gfx::draw_line_2d_pixels(
                arrow_center, arrow_center + arrow_offset * glm::vec2(+1, 1),
                {
                    .color        = Colors::BLACK,
                    .depth        = transform->depth - 0.001f,
                    .anchor_point = Gfx::AnchorPoint::TopLeft,
                    .line_width   = 2.0f,
                });
            Gfx::draw_line_2d_pixels(
                arrow_center, arrow_center + arrow_offset * glm::vec2(-1, 1),
                {
                    .color        = Colors::BLACK,
                    .depth        = transform->depth - 0.001f,
                    .anchor_point = Gfx::AnchorPoint::TopLeft,
                    .line_width   = 2.0f,
                });

            // Render the list if necessary
            Gfx::push_clip_rect(
                top_left + glm::vec2(0, combobox->button_height),
                bottom_right - top_left - glm::vec2(0, combobox->button_height));
            size_t start_index = 0 + static_cast<size_t>(combobox->current_scroll_position / combobox->item_height);
            size_t end_index   = start_index + static_cast<size_t>(combobox->list_height / combobox->item_height) + 1;
            if (abs(transform->bottom_right.y - transform->top_left.y - combobox->button_height) > 1.0f) {
                for (size_t i = start_index; i <= end_index; ++i) {
                    // Stop if end of list was reached
                    if (i >= combobox->list_items.size()) break;

                    // Get transform information
                    box_top_left = top_left + glm::vec2(
                                                  0, combobox->button_height + (combobox->item_height * static_cast<float>(i)) -
                                                         combobox->current_scroll_position);
                    box_bottom_right = glm::vec2(bottom_right.x, box_top_left.y + combobox->item_height);
                    text_offset      = {8, combobox->item_height / 2.0f};

                    // Determine a nice color based on what the mouse is doing
                    glm::vec4 color = {1, 1, 1, 1};

                    // If this is the currently selected entry, darken it a bit
                    if (static_cast<int>(i) == combobox->current_selected_index) {
                        color *= 0.8f;
                    }

                    // Create a hitbox for the current item
                    Hitbox curr_item_hitbox{box_top_left + scene.top_left, box_bottom_right + scene.top_left};

                    // If the mouse is over it, change the color based on the mouse
                    if (curr_item_hitbox.intersects(Input::mouse_position_pixels())) {
                        if (multi_hitbox->click_states[1] == ClickState::hover) {
                            color *= 0.9f;
                        } else if (
                            multi_hitbox->click_states[1] == ClickState::click &&
                            Input::mouse_button_pressed(Input::MouseButton::Left)) {
                            // This is a bit cursed, but it'll have to do
                            // We will actually update the combobox selected index in the rendering code, since we already do a
                            // ton of logic here to figure out where the mouse is anyway
                            color *= 0.7f;
                            combobox->current_selected_index = static_cast<int>(i);
                            combobox->is_list_open           = false;
                            value->set<double>(static_cast<double>(i));
                        }
                    }

                    // Draw the boxes
                    Gfx::draw_rectangle_2d_pixels(
                        box_top_left + glm::vec2(+1, 0), box_bottom_right + glm::vec2(-1, -1),
                        {.color = color, .depth = -0.999f, .anchor_point = Gfx::AnchorPoint::TopLeft});
                    Gfx::draw_rectangle_2d_pixels(
                        box_top_left, box_bottom_right,
                        {
                            .color               = Colors::BLACK,
                            .depth               = -0.999f,
                            .anchor_point        = Gfx::AnchorPoint::TopLeft,
                            .shape_outline_width = 1.0f,
                        });
                    Gfx::draw_text_pixels(
                        combobox->list_items[i].c_str(),
                        Gfx::TextDrawParams{
                            .transform =
                                {.position = glm::vec3(box_top_left + text_offset, -1.0f), .scale = {2.0f, 2.0f, 1.0f}},
                            .position_anchor = Gfx::AnchorPoint::TopLeft,
                            .text_anchor     = Gfx::AnchorPoint::Left,
                            .color           = Colors::BLACK,
                        });
                }
            }
            Gfx::pop_clip_rect();
        }

        // Box
        for (const auto entity: scene.view<UI::Transform, Box>()) {
            auto* transform      = scene.get_component<UI::Transform>(entity);
            auto* box            = scene.get_component<Box>(entity);
            auto* mouse_interact = scene.get_component<MouseInteract>(entity);

            // Hovering and clicking affects color
            float multiply = 1.0f;
            if (mouse_interact) {
                switch (mouse_interact->state) {
                case ClickState::hover: multiply = 0.8f; break;
                case ClickState::click: multiply = 0.6f; break;
                default: break;
                }
            }

            const glm::vec2 tl = Gfx::anchor_offset_pixels(transform->top_left, transform->anchor, scene.panel_size);
            const glm::vec2 br = Gfx::anchor_offset_pixels(transform->bottom_right, transform->anchor, scene.panel_size);
            if (box->color_inner.a > 0.0f) {
                Gfx::draw_rectangle_2d_pixels(
                    tl, br,
                    {
                        .color               = box->color_inner * multiply,
                        .depth               = transform->depth + 0.001f,
                        .anchor_point        = Gfx::AnchorPoint::TopLeft,
                        .shape_outline_width = 0.0f,
                    });
            }
            if (box->color_outer.a > 0.0f) {
                Gfx::draw_rectangle_2d_pixels(
                    tl, br,
                    {
                        .color               = box->color_outer * multiply,
                        .depth               = transform->depth,
                        .anchor_point        = Gfx::AnchorPoint::TopLeft,
                        .shape_outline_width = 1.0f,
                    });
            }
        }
    }

    inline void system_comp_mouse_interact(Scene& scene) {
        // Loop over all MouseInteract components, and handle the state. In this loop we also handle click events since
        // that's literally 2 extra lines of code
        for (const auto entity: scene.view<UI::Transform, MouseInteract>()) {
            const auto* transform = scene.get_component<UI::Transform>(entity);
            const auto* clickable = scene.get_component<Clickable>(entity);
            const auto* function  = scene.get_component<Function>(entity);
            auto* mouse_interact  = scene.get_component<MouseInteract>(entity);

            // Get mouse position, and get an actual correct top-left and bottom-right
            const glm::vec2 mouse_pos = Input::mouse_position_pixels();
            glm::vec2 tl_ =
                Gfx::anchor_offset_pixels(transform->top_left + scene.top_left, transform->anchor, scene.panel_size);
            glm::vec2 br_ =
                Gfx::anchor_offset_pixels(transform->bottom_right + scene.top_left, transform->anchor, scene.panel_size);
            const glm::vec2 tl       = {std::min(tl_.x, br_.x), std::min(tl_.y, br_.y)};
            const glm::vec2 br       = {std::max(tl_.x, br_.x), std::max(tl_.y, br_.y)};
            const glm::vec2 scene_tl = scene.top_left;
            const glm::vec2 scene_br = scene.top_left + scene.panel_size;

            // Determine whether the mouse is inside the component's bounding box
            const bool is_inside_scene = mouse_pos.x >= scene_tl.x && mouse_pos.y >= scene_tl.y && mouse_pos.x <= scene_br.x &&
                                         mouse_pos.y <= scene_br.y;
            const bool is_inside_bb =
                (is_inside_scene) && (mouse_pos.x >= tl.x && mouse_pos.y >= tl.y && mouse_pos.x <= br.x && mouse_pos.y <= br.y);

            // If the element hasn't been clicked
            if (mouse_interact->state != ClickState::click && Input::mouse_button_held(Input::MouseButton::Left) == false) {
                // If the mouse cursor is inside the UI component's bounding box, set the ClickState to hover
                if (is_inside_bb) {
                    mouse_interact->state = ClickState::hover;
                }
                // Otherwise set it to idle
                else {
                    mouse_interact->state = ClickState::idle;
                }
            }
            // If we're hovering over the element and we click, set the ClickState to clicking
            auto* value     = scene.get_component<Value>(entity);
            auto* slider    = scene.get_component<Slider>(entity);
            auto* draggable = scene.get_component<Draggable>(entity);
            auto* range     = scene.get_component<NumberRange>(entity);

            if (Input::mouse_button_held(Input::MouseButton::Left)) {
                if (mouse_interact->state == ClickState::hover) {
                    mouse_interact->state = ClickState::click;
                }
            }
            // If we release the mouse while this element is in click state,
            if (Input::mouse_button_released(Input::MouseButton::Left) && mouse_interact->state == ClickState::click) {
                // and the mouse is still on the component, and the component is clickable
                if (is_inside_bb && clickable && function) {
                    // Call the function of this clickable
                    function->on_click();
                }
                // Reset the MouseInteract state back to idle
                mouse_interact->state = ClickState::idle;
                Gfx::set_mouse_visible(true);

                if (value && slider && draggable) {
                    const double value_normalized = (value->get_as_ref<double>() - range->min) / (range->max - range->min);
                    const double slider_pos       = (value_normalized * std::max(slider->curr_bar_length, 1.0f));

                    if (draggable->is_horizontal) {
                        Input::move_mouse(
                            {slider_pos + br.x - slider->curr_bar_length, Input::mouse_position_pixels().y},
                            Input::MoveMouseMode::Absolute);
                    } else {
                        Input::move_mouse(
                            {Input::mouse_position_pixels().x, slider_pos + br.y - slider->curr_bar_length},
                            Input::MoveMouseMode::Absolute);
                    }
                }
            }

            // If we're hovering over the element and we middle click, AND the component has a value, set that value to default
            if (Input::mouse_button_pressed(Input::MouseButton::Middle) && value && range &&
                mouse_interact->state == ClickState::hover) {
                if (value->type == VarType::float64) {
                    value->set<double>(range->default_value);
                }
            }
            // If we're hovering over the element, display grabby hand cursor
            if ((mouse_interact->state == ClickState::hover || mouse_interact->state == ClickState::click)) {
                Gfx::set_cursor_mode(Gfx::CursorMode::Hand);
            }
        }

        // Handle multi-hitbox components
        for (const auto entity: scene.view<UI::Transform, MultiHitbox>()) {
            const auto* transform = scene.get_component<UI::Transform>(entity);
            auto* multi_hitbox    = scene.get_component<MultiHitbox>(entity);

            glm::vec2 top_left =
                Gfx::anchor_offset_pixels(transform->top_left + scene.top_left, transform->anchor, scene.panel_size);

            // Check for each hitbox
            for (size_t i = 0; i < multi_hitbox->n_hitboxes; ++i) {
                // Transform the hitbox from local space to window space
                Hitbox hitbox = multi_hitbox->hitboxes[i];
                hitbox.top_left += top_left;
                hitbox.bottom_right += top_left;

                // See if it intersects
                if (hitbox.intersects(Input::mouse_position_pixels())) {
                    // If the user is clicking
                    if (Input::mouse_button_held(Input::MouseButton::Left)) {
                        multi_hitbox->click_states[i] = ClickState::click;
                    }

                    // Otherwise hover
                    else {
                        multi_hitbox->click_states[i] = ClickState::hover;
                    }
                } else {
                    multi_hitbox->click_states[i] = ClickState::idle;
                }
            }
        }
    }

    inline void system_comp_draggable_clickable(Scene& scene) {
        // Handle draggable components like sliders, numberboxes,
        for (const auto entity: scene.view<Value, Draggable, MouseInteract, NumberRange>()) {
            const auto* mouse_interact = scene.get_component<MouseInteract>(entity);
            auto* value                = scene.get_component<Value>(entity);
            const auto* number_range   = scene.get_component<NumberRange>(entity);
            const auto* draggable      = scene.get_component<Draggable>(entity);
            const auto* slider         = scene.get_component<Slider>(entity);

            // If the component is being dragged
            if (mouse_interact->state == ClickState::click) {
                // Get a reference to the value
                double& val          = value->get_as_ref<double>();
                const double old_val = val;

                // Map the mouse movement to the value
                const auto step =
                    (slider && (Input::key_held(Input::Key::LeftControl) || Input::key_held(Input::Key::RightControl)))
                        ? number_range->step_fine
                        : number_range->step;

                if (draggable) {
                    if (draggable->is_horizontal) {
                        val += static_cast<double>(Input::mouse_movement_pixels().x) * step;
                    } else {
                        val -= static_cast<double>(Input::mouse_movement_pixels().y) * step;
                    }
                }

                // Clamp the value to the bounds
                val = std::max(val, number_range->min);
                val = std::min(val, number_range->max);

                // Handle changed variable, since we didn't use set
                value->has_changed = (val != old_val);

                // Make the mouse invisible
                Gfx::set_mouse_visible(false);
            }
        }

        // Handle scrollable components like sliders, numberboxes
        for (const auto entity: scene.view<Value, Scrollable, MouseInteract, NumberRange>()) {
            const auto* mouse_interact = scene.get_component<MouseInteract>(entity);
            auto* value                = scene.get_component<Value>(entity);
            const auto* number_range   = scene.get_component<NumberRange>(entity);

            // If the component is hovered over
            if (mouse_interact->state == ClickState::hover) {
                // Get a reference to the value
                double& val          = value->get_as_ref<double>();
                const double old_val = val;

                // Map the vertical mouse scroll to the value
                const auto step = (Input::key_held(Input::Key::LeftControl) || Input::key_held(Input::Key::RightControl))
                                    ? number_range->step_fine
                                    : number_range->step;

                val += static_cast<double>(Input::mouse_scroll().y) * step;

                // Clamp the value to the bounds
                val = std::max(val, number_range->min);
                val = std::min(val, number_range->max);

                // Handle changed variable, since we didn't use set
                value->has_changed = (val != old_val);
            }
        }
    }

    inline void system_comp_radio_buttons(Scene& scene) {
        // Handle radio buttons
        for (const auto entity: scene.view<UI::Transform, Value, RadioButton, MultiHitbox>()) {
            const auto* transform      = scene.get_component<UI::Transform>(entity);
            auto* value                = scene.get_component<Value>(entity);
            auto* radio_button         = scene.get_component<RadioButton>(entity);
            const auto* mouse_interact = scene.get_component<MouseInteract>(entity);
            const auto* multi_hitbox   = scene.get_component<MultiHitbox>(entity);

            // Make sure it also has a mouse interact component
            if (mouse_interact == nullptr) {
                continue;
            }

            // If the component is clicked on in general
            if (Input::mouse_button_pressed(Input::MouseButton::Left) && mouse_interact->state == ClickState::click) {
                // Check for each hitbox
                for (size_t i = 0; i < multi_hitbox->n_hitboxes; ++i) {
                    // Transform the hitbox from local space to window space
                    Hitbox hitbox = multi_hitbox->hitboxes[i];
                    hitbox.top_left +=
                        Gfx::anchor_offset_pixels(transform->top_left + scene.top_left, transform->anchor, scene.panel_size);
                    hitbox.bottom_right +=
                        Gfx::anchor_offset_pixels(transform->top_left + scene.top_left, transform->anchor, scene.panel_size);

                    // See if it intersects
                    if (hitbox.intersects(Input::mouse_position_pixels())) {
                        // If so, select that value
                        radio_button->current_selected_index = i;
                        value->set<double>(static_cast<double>(i));
                    }
                }
            }
        }
    }

    inline void system_comp_combobox(Scene& scene, const float delta_time, bool& combobox_handled) {
        // Handle combobox
        for (const auto entity: scene.view<UI::Transform, Value, Combobox, MultiHitbox>()) {
            auto* transform            = scene.get_component<UI::Transform>(entity);
            auto* combobox             = scene.get_component<Combobox>(entity);
            const auto* mouse_interact = scene.get_component<MouseInteract>(entity);
            const auto* multi_hitbox   = scene.get_component<MultiHitbox>(entity);
            auto* value                = scene.get_component<Value>(entity);

            // Make sure it also has a mouse interact component
            if (mouse_interact == nullptr) {
                continue;
            }

            // If the mouse is clicked on in general
            if (Input::mouse_button_pressed(Input::MouseButton::Left)) {
                // If it's the top part of the combobox, toggle it
                if (multi_hitbox->click_states[0] == ClickState::click) {
                    combobox->is_list_open = !combobox->is_list_open;
                    combobox_handled       = true;
                }

                // If it's outside the combobox in general, close it
                else if (combobox->is_list_open) {
                    combobox->is_list_open = false;
                    combobox_handled       = true;
                }
            }

            // Make sure we have full focus when the mouse is on it
            if (mouse_interact->state != ClickState::idle) {
                combobox_handled = true;
            }

            // Handle scrolling
            float scroll = Input::mouse_scroll().y;
            if (scroll != 0) {
                // If we are hovered over the list, scroll the list
                if (multi_hitbox->click_states[1] == ClickState::hover) {
                    combobox->target_scroll_position -= scroll * combobox->item_height * 1.25f;
                    float min = 0.0f;
                    float max = combobox->item_height * static_cast<float>(combobox->list_items.size()) - combobox->list_height;
                    max       = std::max(min, max);
                    combobox->target_scroll_position = std::clamp(combobox->target_scroll_position, min, max);
                }

                // Otherwise if we are hovered over the button, change the index
                if (multi_hitbox->click_states[0] == ClickState::hover) {
                    combobox->current_selected_index -= static_cast<int>(scroll);
                    combobox->target_scroll_position =
                        (static_cast<float>(combobox->current_selected_index) - 0.5f) * combobox->item_height;
                    float min = 0.0f;
                    float max = combobox->item_height * static_cast<float>(combobox->list_items.size()) - combobox->list_height;
                    max       = std::max(min, max);
                    combobox->target_scroll_position = std::clamp(combobox->target_scroll_position, min, max);
                    combobox->current_selected_index =
                        std::clamp(combobox->current_selected_index, 0, static_cast<int>(combobox->list_items.size()) - 1);
                    value->set<double>(combobox->current_selected_index);
                }
            }

            // Handle transform
            const float target_bottom = transform->top_left.y + combobox->button_height +
                                        ((combobox->is_list_open ? 1.0f : 0.0f) * combobox->list_height);
            transform->bottom_right.y =
                std::lerp(transform->bottom_right.y, target_bottom, 1.0f - pow(2.0f, -delta_time * 75.0f));

            // Interpolate towards the scroll
            combobox->current_scroll_position = std::lerp(
                combobox->current_scroll_position, combobox->target_scroll_position, 1.0f - pow(2.0f, -delta_time * 25.0f));
        }
    }

    inline void update_entities_render(Scene& scene) {
        // Render sprites
        system_comp_sprite(scene);
        system_comp_special_render(scene);

        // Render text
        system_comp_text(scene);
    }

    inline void update_entities_input(Scene& scene, float delta_time, bool do_mouse_interact) {
        // Handle clickable components
        if (do_mouse_interact) system_comp_mouse_interact(scene);

        // Handle comboboxes - special case: if a combobox is interacted with, don't handle any other ones
        bool combobox_handled = false;
        system_comp_combobox(scene, delta_time, combobox_handled);

        if (combobox_handled == false) {
            // Handle other mouse interactable components
            system_comp_draggable_clickable(scene);

            // Handle radio buttons
            system_comp_radio_buttons(scene);
        }

        // Handle value changes
        for (const auto entity: scene.view<Value, Function>()) {
            auto* value          = scene.get_component<Value>(entity);
            const auto* function = scene.get_component<Function>(entity);
            if (value->has_changed) {
                value->has_changed = false;
                function->on_click();
            }
        }

        // Debug
#ifdef _DEBUG
        for (const auto entity: scene.view<UI::Transform>()) {
            const auto* transform = scene.get_component<UI::Transform>(entity);
            // todo
            // renderer.draw_box_line(
            //     *transform, transform->top_left, transform->bottom_right, {1, 0, 1, 1}, 1, 0, transform->anchor);
        }
#endif
        // Gfx::set_clip_rect();
    }
} // namespace UI
