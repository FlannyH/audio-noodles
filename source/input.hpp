#pragma once
#include <GLFW/glfw3.h>
#include <glm/vec2.hpp>

namespace Gfx {
    class Device;
}

namespace Input {
    enum class Key {
        Invalid,

        A,
        B,
        C,
        D,
        E,
        F,
        G,
        H,
        I,
        J,
        K,
        L,
        M,
        N,
        O,
        P,
        Q,
        R,
        S,
        T,
        U,
        V,
        W,
        X,
        Y,
        Z,

        Space,
        Escape,
        Enter,
        Tab,
        LeftShift,
        LeftControl,
        LeftAlt,
        RightShift,
        RightControl,
        RightAlt,

        _0,
        _1,
        _2,
        _3,
        _4,
        _5,
        _6,
        _7,
        _8,
        _9,
        Up,
        Down,
        Left,
        Right,

        F1,
        F2,
        F3,
        F4,
        F5,
        F6,
        F7,
        F8,
        F9,
        F10,
        F11,
        F12,

        KeyCount
    };

    enum class MouseButton {
        Left,
        Right,
        Middle,

        ButtonCount,
    };

    struct InputData {
        double mouse_x = 0.0;
        double mouse_y = 0.0;
        double mouse_scroll_x = 0.0;
        double mouse_scroll_y = 0.0;
        bool mouse_buttons[(size_t)MouseButton::ButtonCount] = {0};
        bool keys[(size_t)Key::KeyCount] = {0};
    };

    void update();

    // Keyboard
    bool key_held(Key key);
    bool key_pressed(Key key);
    bool key_released(Key key);

    // Mouse
    bool mouse_button_held(MouseButton mouse_button);
    bool mouse_button_pressed(MouseButton mouse_button);
    bool mouse_button_released(MouseButton mouse_button);
    glm::vec2 mouse_scroll();
    glm::vec2 mouse_position();
    glm::vec2 mouse_position_pixels();
    glm::vec2 mouse_movement();
    glm::vec2 mouse_movement_pixels();

    InputData* get_ptr_incoming();
}; // namespace Input
