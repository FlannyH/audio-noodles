#pragma once
#include "device.hpp"
#include "resource.hpp"
#include "transform.hpp"
#include "../colors.hpp"
#include <map>
#include <string>
#include <memory>

namespace Gfx {
    enum class RenderAPI { OpenGL };

    enum class AnchorPoint { TopLeft = 0, Top, TopRight, Left, Center, Right, BottomLeft, Bottom, BottomRight };

    enum class RenderInfoType { None = 0, Raster, Blit };

    struct DrawParams {
        glm::vec4 color          = {1, 1, 1, 1};
        float depth              = 0.0f;
        AnchorPoint anchor_point = AnchorPoint::Center;
        // Line specific parameters
        float line_width = 0.001f; // Will always result in at least 1 pixel wide lines

        // Rectangle specific parameters
        glm::vec2 texcoord_tl     = glm::vec2(0.0f, 0.0f);
        glm::vec2 texcoord_br     = glm::vec2(1.0f, 1.0f);
        float shape_outline_width = 0.0f; // Values equal to or less than 0.0f will make the shape filled.
                                          // Otherwise, it will always result in at least 1 pixel wide lines.
        ResourceID texture = ResourceID::invalid();
    };

    struct DrawParams3D {
        glm::vec4 color = {1, 1, 1, 1};
    };

    struct TextureCreationParams {
        PixelFormat format  = PixelFormat::Invalid;
        TextureType type    = TextureType::Invalid;
        size_t width        = 0;
        size_t height       = 0;
        size_t depth        = 1; // Only applicable to 3D textures and texture arrays
        bool is_framebuffer = false;
        void* data          = nullptr;
    };

    struct PosTexcoord {
        glm::vec2 pos;
        glm::vec2 texcoord;
    };

    struct PixelRect {
        glm::i16vec2 top_left;
        glm::i16vec2 size;
    };

    struct Font {
        std::vector<PixelRect> glyph_rects;
        std::map<wchar_t, std::vector<int>> wchar_mapping;
        glm::i16vec2 glyph_cell_size = glm::i16vec2(0);
        ResourceID tex_id            = ResourceID::invalid();
    };

    struct TextDrawParams {
        Transform transform{};
        AnchorPoint position_anchor = AnchorPoint::Center;
        AnchorPoint text_anchor;
        Color color;
    };

    // Renderer is responsible for creating a window, rendering graphics, and handling input
    // Meta
    bool init(
        RenderAPI api = RenderAPI::OpenGL, int window_width = 1280, int window_height = 720,
        const char* title = "Audio Noodles");
    bool should_stay_open();
    glm::vec2 get_window_size();
    glm::vec2 get_viewport_size();
    float get_delta_time();
    float get_fps();
    void set_mouse_visible(bool visible);
    void set_cursor_mode(CursorMode cursor_mode);
    void set_render_target(ResourceID render_target = ResourceID::invalid());

    // Rendering
    void begin_frame();
    void end_frame();
    void push_clip_rect(glm::ivec2 top_left = {0.0f, 0.0f}, glm::ivec2 size = {99999.0f, 99999.0f});
    void pop_clip_rect();
    void set_viewport(glm::ivec2 top_left = {0.0f, 0.0f}, glm::ivec2 size = {99999.0f, 99999.0f});

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
    glm::vec2 anchor_offset(glm::vec2 top_left, Gfx::AnchorPoint anchor);
    glm::vec2 anchor_offset_pixels(
        glm::vec2 top_left, Gfx::AnchorPoint anchor,
        glm::vec2 anchor_size = glm::vec2(0.0f, 0.0f)); // if anchor_size == 0, it uses the window size
    void draw_circle_2d_pixels(glm::vec2 center, glm::vec2 size, DrawParams draw_params = {});
    void blit_pixels(ResourceID src, ResourceID dest, glm::ivec2 size, glm::ivec2 dest_tl = {0, 0}, glm::ivec2 src_tl = {0, 0});

    // Text
    std::shared_ptr<Font> load_font(const std::string& path);
    float get_font_height();
    float get_font_max_width();
    void draw_text_pixels(const std::string& text, TextDrawParams params);
    void draw_text_pixels(const wchar_t* text, TextDrawParams params);

    // Resources
    ResourceID create_buffer(const std::string_view& name, const size_t size, const void* data = nullptr);
    ResourceID create_texture_from_data(TextureCreationParams params);
    ResourceID create_texture2d_from_file(const std::string& path);
    void resize_texture(ResourceID id, glm::ivec3 new_resolution);
} // namespace Gfx
