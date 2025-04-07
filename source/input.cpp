#include "input.hpp"
#include <cstring>
#include "graphics/renderer.hpp"

namespace Input {
    // State
    InputData previous{};
    InputData current{};

    // Will be written to from the renderer
    InputData incoming{};

    void update() {
        // Copy new state to curr, and curr to prev - this way curr and prev are stable across the entire frame, while
        // new is being updated by the callback
        memcpy(previous.keys, current.keys, sizeof(previous.keys));
        memcpy(current.keys, incoming.keys, sizeof(previous.keys));
        memcpy(previous.mouse_buttons, current.mouse_buttons, sizeof(previous.mouse_buttons));
        memcpy(current.mouse_buttons, incoming.mouse_buttons, sizeof(previous.mouse_buttons));
        previous.mouse_x        = current.mouse_x;
        current.mouse_x         = incoming.mouse_x;
        previous.mouse_y        = current.mouse_y;
        current.mouse_y         = incoming.mouse_y;
        previous.mouse_scroll_x = current.mouse_scroll_x;
        current.mouse_scroll_x  = incoming.mouse_scroll_x;
        previous.mouse_scroll_y = current.mouse_scroll_y;
        current.mouse_scroll_y  = incoming.mouse_scroll_y;
    }

    bool key_held(Key key) { return current.keys[(size_t)key]; }

    bool key_pressed(Key key) {
        auto curr = current.keys[(size_t)key];
        auto prev = previous.keys[(size_t)key];
        return curr && !prev;
    }

    bool key_released(Key key) {
        auto curr = current.keys[(size_t)key];
        auto prev = previous.keys[(size_t)key];
        return prev && !curr;
    }

    bool mouse_button_held(MouseButton mouse_button) { return current.mouse_buttons[(size_t)mouse_button]; }

    bool mouse_button_pressed(MouseButton mouse_button) {
        auto curr = current.mouse_buttons[(size_t)mouse_button];
        auto prev = previous.mouse_buttons[(size_t)mouse_button];
        return curr && !prev;
    }

    bool mouse_button_released(MouseButton mouse_button) {
        auto curr = current.mouse_buttons[(size_t)mouse_button];
        auto prev = previous.mouse_buttons[(size_t)mouse_button];
        return prev && !curr;
    }

    glm::vec2 mouse_scroll() {
        return glm::vec2(current.mouse_scroll_x - previous.mouse_scroll_x, current.mouse_scroll_y - previous.mouse_scroll_y);
    }

    glm::vec2 mouse_position() {
        const glm::vec2 window_resolution = Gfx::get_window_size();
        return (glm::vec2(current.mouse_x, current.mouse_y) / window_resolution) - 0.5f;
    }

    glm::vec2 mouse_position_pixels() { return glm::vec2(current.mouse_x, current.mouse_y); }

    glm::vec2 mouse_movement() {
        const glm::vec2 window_resolution = Gfx::get_window_size();
        const glm::vec2 movement_pixels(current.mouse_x - previous.mouse_x, current.mouse_y - previous.mouse_y);
        return (movement_pixels / window_resolution);
    }

    glm::vec2 mouse_movement_pixels() {
        return glm::vec2(current.mouse_x - previous.mouse_x, current.mouse_y - previous.mouse_y);
    }

    InputData* get_ptr_incoming() { return &incoming; }
} // namespace Input
