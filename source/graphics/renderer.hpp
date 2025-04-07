#pragma once
#include "device.hpp"
#include "resource.hpp"
#include <map>
#include <string>

namespace Gfx {
    enum class RenderAPI { OpenGL };

    enum class AnchorPoint { TopLeft = 0, Top, TopRight, Left, Center, Right, BottomLeft, Bottom, BottomRight };

    struct DrawParams {
        glm::vec4 color = {1, 1, 1, 1};
        float depth = 0.0f;
        AnchorPoint anchor_point = AnchorPoint::Center;
        // Line specific parameters
        float line_width = 0.001f; // Will always result in at least 1 pixel wide lines

        // Rectangle specific parameters
        glm::vec2 texcoord_tl = glm::vec2(0.0f, 0.0f);
        glm::vec2 texcoord_br = glm::vec2(1.0f, 1.0f);
        float rectangle_outline_width = 0.0f; // Values equal to or less than 0.0f will make the rectangle filled.
                                              // Otherwise, it will always result in at least 1 pixel wide lines.
        ResourceID texture = ResourceID::invalid();
    };

    struct DrawParams3D {
        glm::vec4 color = {1, 1, 1, 1};
    };

    struct TextureCreationParams {
        PixelFormat format = PixelFormat::Invalid;
        TextureType type = TextureType::Invalid;
        size_t width = 0;
        size_t height = 0;
        size_t depth = 1; // Only applicable to 3D textures and texture arrays
        void* data = nullptr;
    };

    struct PosTexcoord {
        glm::vec2 pos;
        glm::vec2 texcoord;
    };

    // Renderer is responsible for creating a window, rendering graphics, and handling input
    // Meta
    bool init(
        RenderAPI api = RenderAPI::OpenGL, int window_width = 1280, int window_height = 720,
        const char* title = "Audio Noodles");
    bool should_stay_open();
    glm::vec2 get_window_size();
    float get_delta_time();
    float get_fps();

    // Rendering
    void begin_frame();
    void end_frame();

    // 2D rendering in normalized device coordinates (-1.0 to 1.0)
    void draw_line_2d(glm::vec2 v0, glm::vec2 v1, const DrawParams& draw_params = {});
    void draw_triangle_2d(PosTexcoord v0, PosTexcoord v1, PosTexcoord v2, const DrawParams& draw_params = {});
    void draw_quad_2d(PosTexcoord v0, PosTexcoord v1, PosTexcoord v2, PosTexcoord v3, const DrawParams& draw_params = {});
    void draw_rectangle_2d(glm::vec2 top_left, glm::vec2 bottom_right, const DrawParams& draw_params = {});

    // 2D rendering in pixel coordinates
    void draw_line_2d_pixels(glm::vec2 v0, glm::vec2 v1, DrawParams draw_params = {});
    void draw_triangle_2d_pixels(PosTexcoord v0, PosTexcoord v1, PosTexcoord v2, const DrawParams& draw_params = {});
    void
    draw_quad_2d_pixels(PosTexcoord v0, PosTexcoord v1, PosTexcoord v2, PosTexcoord v3, const DrawParams& draw_params = {});
    void draw_rectangle_2d_pixels(glm::vec2 top_left, glm::vec2 bottom_right, DrawParams draw_params = {});

    // Resources
    ResourceID create_buffer(const std::string_view& name, const size_t size, const void* data = nullptr);
    ResourceID create_texture_from_data(TextureCreationParams params);
    ResourceID create_texture2d_from_file(const std::string& path);
} // namespace Gfx
