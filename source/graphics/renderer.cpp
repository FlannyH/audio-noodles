#include "renderer.hpp"
#include "resource.hpp"
#include "../log.hpp"
#include "opengl/device_opengl.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <glm/geometric.hpp>

namespace Gfx {
    Device* device        = nullptr; // Will be initialized to a child class of Device (e.g. DeviceOpenGL)
    glm::vec2 window_size = glm::vec2(0.0f);
    float aspect_ratio    = 1.0f;

    // 2D rendering
    ResourceID pipeline_2d = {0, 0};
    std::map<uint32_t, std::vector<Vertex2D>> render_queue_2d; // maps textures to vertex buffers
    ResourceID render_queue_2d_gpu_buffer = {0, 0};

    // Text
    std::shared_ptr<Font> font;

    bool init(RenderAPI api, int window_width, int window_height, const char* title) {
        // Create rendering device
        switch (api) {
        case RenderAPI::OpenGL: device = new DeviceOpenGL(window_width, window_height, title); break;
        default: return false;
        }

        // Create common buffers
        constexpr size_t render_queue_max_vertex_count = 65536 * 16;
        constexpr size_t render_data_3d_buffer_size    = 128 * 1024;
        render_queue_2d_gpu_buffer =
            device->create_buffer("Render queue 2D GPU buffer", sizeof(Vertex2D) * render_queue_max_vertex_count);

        // Create common shader pipelines
        pipeline_2d = device->load_pipeline_raster("assets/shaders/2d.vsh", "assets/shaders/2d.psh");

        // Load font
        font = load_font("assets/textures/font.png");

        if (api == RenderAPI::OpenGL) LOG(Info, "Renderer initialized (OpenGL)");

        return true;
    }

    bool should_stay_open() { return device->should_stay_open(); }

    glm::vec2 get_window_size() { return window_size; }

    float get_delta_time() { return device->get_delta_time(); }

    float get_fps() { return 1.0f / get_delta_time(); }

    void begin_frame() {
        // Update window size
        int w, h;
        device->get_window_size(w, h);
        window_size  = {(float)w, (float)h};
        aspect_ratio = window_size.x / window_size.y;

        render_queue_2d.clear();
        device->handle_resize(w, h);
        device->begin_frame();
        device->clear_framebuffer(glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
    }

    void end_frame() {
        if (!render_queue_2d.empty()) {
            for (const auto& [key, value]: render_queue_2d) {
                device->upload_data_to_buffer(render_queue_2d_gpu_buffer, 0, sizeof(Vertex2D) * value.size(), value.data());
                device->begin_raster_pass(pipeline_2d);
                device->bind_resources({{render_queue_2d_gpu_buffer, 0}});
                device->bind_texture(0, ResourceID::from_u32(key));
                device->execute_raster(value.size());
                device->end_raster_pass();
            }
        }

        device->end_frame();
    }

    void draw_line_2d(glm::vec2 a, glm::vec2 b, const DrawParams& draw_params) {
        float width = draw_params.line_width;
        if (width * window_size.y < 0.5f) {
            width = 1.0f / window_size.y;
        }

        // Figure out rectangle to draw
        const glm::vec2 direction               = b - a;
        const glm::vec2 perpendicular           = glm::normalize(glm::vec2(direction.y, -direction.x));
        const glm::vec2 corrected_perpendicular = (perpendicular * width) * glm::vec2(1.0f / aspect_ratio, 1.0f);
        const glm::vec2 v0                      = a - corrected_perpendicular;
        const glm::vec2 v1                      = a + corrected_perpendicular;
        const glm::vec2 v2                      = b + corrected_perpendicular;
        const glm::vec2 v3                      = b - corrected_perpendicular;
        draw_quad_2d({v0}, {v1}, {v2}, {v3}, draw_params);
    }

    void draw_triangle_2d(PosTexcoord v0, PosTexcoord v1, PosTexcoord v2, const DrawParams& draw_params) {
        constexpr glm::vec2 anchor_offsets[] = {
            glm::vec2(-1.0f, 1.0f),  // TopLeft
            glm::vec2(0.0f, 1.0f),   // Top
            glm::vec2(1.0f, 1.0f),   // TopRight
            glm::vec2(-1.0f, 0.0f),  // Left
            glm::vec2(0.0f, 0.0f),   // Center
            glm::vec2(1.0f, 0.0f),   // Right
            glm::vec2(-1.0f, -1.0f), // BottomLeft
            glm::vec2(0.0f, -1.0f),  // Bottom
            glm::vec2(1.0f, -1.0f),  // BottomRight
        };
        render_queue_2d[draw_params.texture.as_u32()].push_back(Vertex2D(
            (v0.pos * 2.0f) + anchor_offsets[(size_t)draw_params.anchor_point], draw_params.depth, draw_params.color,
            v0.texcoord, draw_params.texture));
        render_queue_2d[draw_params.texture.as_u32()].push_back(Vertex2D(
            (v1.pos * 2.0f) + anchor_offsets[(size_t)draw_params.anchor_point], draw_params.depth, draw_params.color,
            v1.texcoord, draw_params.texture));
        render_queue_2d[draw_params.texture.as_u32()].push_back(Vertex2D(
            (v2.pos * 2.0f) + anchor_offsets[(size_t)draw_params.anchor_point], draw_params.depth, draw_params.color,
            v2.texcoord, draw_params.texture));
    }

    void draw_quad_2d(PosTexcoord v0, PosTexcoord v1, PosTexcoord v2, PosTexcoord v3, const DrawParams& draw_params) {
        draw_triangle_2d(v0, v1, v2, draw_params);
        draw_triangle_2d(v0, v2, v3, draw_params);
    }

    void draw_rectangle_2d(glm::vec2 top_left, glm::vec2 bottom_right, const DrawParams& draw_params) {
        if (draw_params.rectangle_outline_width <= 0.0f) {
            const glm::vec2 v0(top_left.x, top_left.y);
            const glm::vec2 v1(bottom_right.x, top_left.y);
            const glm::vec2 v2(bottom_right.x, bottom_right.y);
            const glm::vec2 v3(top_left.x, bottom_right.y);
            const glm::vec2 tc0(draw_params.texcoord_tl.x, draw_params.texcoord_tl.y);
            const glm::vec2 tc1(draw_params.texcoord_br.x, draw_params.texcoord_tl.y);
            const glm::vec2 tc2(draw_params.texcoord_br.x, draw_params.texcoord_br.y);
            const glm::vec2 tc3(draw_params.texcoord_tl.x, draw_params.texcoord_br.y);
            draw_quad_2d({v0, tc0}, {v1, tc1}, {v2, tc2}, {v3, tc3}, draw_params);
        } else {
            float x1 = std::min(top_left.x, bottom_right.x) + (2.0f * draw_params.rectangle_outline_width / aspect_ratio);
            float x2 = std::max(top_left.x, bottom_right.x) - (2.0f * draw_params.rectangle_outline_width / aspect_ratio);
            float y1 = std::min(top_left.y, bottom_right.y) + (2.0f * draw_params.rectangle_outline_width);
            float y2 = std::max(top_left.y, bottom_right.y) - (2.0f * draw_params.rectangle_outline_width);

            // Outside coords
            const glm::vec2 v0(top_left.x, top_left.y);
            const glm::vec2 v1(bottom_right.x, top_left.y);
            const glm::vec2 v2(bottom_right.x, bottom_right.y);
            const glm::vec2 v3(top_left.x, bottom_right.y);
            // Inside coords
            const glm::vec2 v4(x1, y1);
            const glm::vec2 v5(x2, y1);
            const glm::vec2 v6(x2, y2);
            const glm::vec2 v7(x1, y2);

            draw_quad_2d({v0}, {v1}, {v5}, {v4}, draw_params); // Top
            draw_quad_2d({v1}, {v2}, {v6}, {v5}, draw_params); // Right
            draw_quad_2d({v7}, {v6}, {v2}, {v3}, draw_params); // Bottom
            draw_quad_2d({v0}, {v4}, {v7}, {v3}, draw_params); // Left
        }
    }

    void draw_line_2d_pixels(glm::vec2 v0, glm::vec2 v1, DrawParams draw_params) {
        draw_params.line_width /= window_size.y;
        draw_line_2d(v0 / window_size, v1 / window_size, draw_params);
    }

    void draw_triangle_2d_pixels(PosTexcoord v0, PosTexcoord v1, PosTexcoord v2, const DrawParams& draw_params) {
        v0.pos /= window_size;
        v1.pos /= window_size;
        v2.pos /= window_size;
        draw_triangle_2d(v0, v1, v2, draw_params);
    }

    void draw_quad_2d_pixels(PosTexcoord v0, PosTexcoord v1, PosTexcoord v2, PosTexcoord v3, const DrawParams& draw_params) {
        v0.pos /= window_size;
        v1.pos /= window_size;
        v2.pos /= window_size;
        v3.pos /= window_size;
        draw_quad_2d(v0, v1, v2, v3, draw_params);
    }

    void draw_rectangle_2d_pixels(glm::vec2 top_left, glm::vec2 bottom_right, DrawParams draw_params) {
        draw_params.rectangle_outline_width /= window_size.y;
        draw_rectangle_2d(top_left / window_size, bottom_right / window_size, draw_params);
    }
    std::shared_ptr<Font> load_font(const std::string& path) {
        std::shared_ptr<Font> new_font = std::make_shared<Font>();

        // todo(lily): un-hardcode the mapping if we decide to expand font rendering
        // Process regular ascii first
        const std::wstring lut =
            L" !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~¡¿";
        for (size_t x = 0; x < lut.size(); x++) {
            new_font->wchar_mapping[lut[x]] = {static_cast<int>(x)};
        }

        // Process the accented characters next
        const std::string ascii_list     = "AAAAACEEEEIIIINOOOOOUUUUY"
                                           "aaaaaceeeeiiiinooooouuuuy";
        const std::wstring accented_list = L"ÁÀÂÃÄÇÉÈÊËÍÌÎÏÑÓÒÔÕÖÚÙÛÜÝ"
                                           L"áàâãäçéèêëíìîïñóòôõöúùûüý";
        const std::string accent_markers = "0124380123012340124301230"
                                           "0124380123012340124301230";
        constexpr int accent_offset      = 0x70 - '0';

        for (size_t x = 0; x < ascii_list.size(); x++) {
            new_font->wchar_mapping[accented_list[x]] = {
                static_cast<int>(lut.find_first_of(ascii_list[x])), accent_markers[x] + accent_offset};
        }

        int w, h;
        uint8_t* data = stbi_load(path.c_str(), &w, &h, nullptr, 4);

        if (!data) {
            LOG(Error, "File not found: \"%s\".", path.c_str());
            return nullptr;
        }

        if (w % 16 != 0 || h % 8 != 0) {
            LOG(Error,
                "Font atlas \"%s\" resolution is not a multiple of 16 in at least one of the texture dimensions!"
                "The font texture must be a 16x16 grid of glyphs. Rejecting font.",
                path.c_str());
            return nullptr;
        }

        const glm::ivec2 glyph_size = {w / 16, h / 8};
        std::array<int, 128> widths;
        widths.fill(glyph_size.x); // Use the max glyph size by default, making a font monospace by default

        // Each glyph may have a dark red RGB(127, 0, 0) line to the right of it to determine its width, in the case of a
        // variable width font
        for (int y = 0; y < 8; y++) {
            const int sample_y = y * glyph_size.y;
            for (int x = 0; x < 16; x++) {
                int width = glyph_size.x;
                for (int glyph_x = 0; glyph_x < glyph_size.x; glyph_x++) {
                    const int sample_x   = (x * glyph_size.x) + glyph_x;
                    const uint32_t pixel = (reinterpret_cast<uint32_t*>(data))[sample_y * w + sample_x];

                    if ((pixel & 0x00FFFFFF) == 0x00007F) {
                        width = glyph_x + 2;
                        break;
                    }
                }
                widths[x + (y * 16)] = width;
            }
        }

        // Remove all the red pixels from the font
        for (int y = 0; y < h; y++) {
            for (int x = 0; x < w; x++) {
                uint32_t& pixel = (reinterpret_cast<uint32_t*>(data))[y * w + x];
                if ((pixel & 0x00FFFFFF) == 0x00007F) {
                    pixel = 0;
                }
            }
        }

        // Populate glyph rectangles
        for (int y = 0; y < 8; ++y) {
            for (int x = 0; x < 16; ++x) {
                new_font->glyph_rects.emplace_back(
                    PixelRect{.top_left = {x * glyph_size.x, y * glyph_size.y}, .size = {widths[x + (y * 16)], glyph_size.y}});
            }
        }

        new_font->glyph_cell_size = glyph_size;

        new_font->tex_id = create_texture_from_data(TextureCreationParams{
            .format = PixelFormat::RGBA_8,
            .type   = TextureType::Single2D,
            .width  = (size_t)w,
            .height = (size_t)h,
            .depth  = 1,
            .data   = data,
        });

        return new_font;
    }

    float get_font_height() { return font->glyph_cell_size.y; }

    void draw_text_pixels(const std::wstring& text, TextDrawParams params) {
        if (!font) return;

        glm::vec2 cur_pos = params.transform.position;

        // Calculate text height, and widths for each line
        float height = static_cast<float>(font->glyph_cell_size.y) * params.transform.scale.y;
        std::vector<float> widths;
        {
            float width = 0;
            for (auto& c: text) {
                if (c == '\n') {
                    widths.push_back(width);
                    width = 0;
                    continue;
                }

                std::vector<int>& wentry = font->wchar_mapping[static_cast<wchar_t>(c)];
                if (!wentry.empty())
                    width += static_cast<float>(font->glyph_rects[wentry[0]].size.x) * params.transform.scale.x;
            }
            widths.push_back(width); // Text may not end with a newline, so we need to push back the last width manually
        }
        height *= static_cast<float>(widths.size());

        constexpr glm::vec2 anchor_offsets[] = {
            {-1, 1},  // top left
            {0, 1},   // top
            {1, 1},   // top right
            {-1, 0},  // left
            {0, 0},   // center
            {1, 0},   // right
            {-1, -1}, // bottom left
            {0, -1},  // bottom
            {1, -1},  // bottom right
        };

        // Calculate offsets based on text anchor point
        std::vector<glm::vec3> offsets(widths.size());
        for (size_t i = 0; i < widths.size(); i++) {
            // Get the offset from the table
            glm::vec3 offset = glm::vec3(anchor_offsets[static_cast<size_t>(params.text_anchor)], 0);

            // We also need to invert the x-axis
            offset.x = -offset.x;

            // Since text by default renders from top_left, we need to offset the offset to the center
            offset.x -= 1;
            offset.y -= 1;

            // We need to scale this to the width and height of the
            offset.x *= widths[i] * 0.5f;
            offset.y *= height * 0.5f;

            // Add it to the offset array
            offsets[i] = offset;
        }

        int width_idx = 0;
        for (auto& c: text) {
            // Handle newline
            if (c == '\n') {
                cur_pos.x = params.transform.position.x;
                cur_pos.y += static_cast<float>(font->glyph_cell_size.y) * params.transform.scale.y;
                width_idx++;
                continue;
            }
            if (c == '\r') {
                cur_pos.x = params.transform.position.x;
                continue;
            }
            if (c == '\t') {
                cur_pos.x += static_cast<float>(font->glyph_cell_size.x * 4);
                continue;
            }

            // Create verts
            std::vector<int>& wentry = font->wchar_mapping[static_cast<wchar_t>(c)];

            for (size_t i = 0; i < wentry.size(); i++) {
                auto wc                 = wentry[i];
                glm::vec4 color_noalpha = params.color * glm::vec4(1, 1, 1, 0);
                glm::vec3 pos_depth =
                    (glm::vec3(cur_pos, params.transform.position.z) + glm::vec3(0, i * 2, 0)) + offsets[width_idx];
                glm::vec2 off_uv     = glm::vec2(wc % 16, wc >> 4) / glm::vec2(16.f, 8.f);
                glm::vec2 glyph_size = glm::vec2(1.f / 16.f, 1.f / 8.f);
                float grid_w_2       = static_cast<float>(font->glyph_cell_size.x) * params.transform.scale.x;
                float grid_h_2       = static_cast<float>(font->glyph_cell_size.y) * params.transform.scale.y;

                draw_rectangle_2d_pixels(
                    pos_depth + glm::vec3(0, 0, 0.0f), pos_depth + glm::vec3(grid_w_2, grid_h_2, 0.0f),
                    DrawParams{
                        .color        = params.color,
                        .depth        = pos_depth.z,
                        .anchor_point = params.position_anchor,
                        .texcoord_tl  = off_uv,
                        .texcoord_br  = off_uv + glyph_size,
                        .texture      = font->tex_id});
            }

            // Move cursor
            if (!wentry.empty())
                cur_pos.x += static_cast<float>(font->glyph_rects[wentry[0]].size.x) * params.transform.scale.x;
        }
    }

    void draw_text_pixels(const std::string& text, TextDrawParams params) {
        draw_text_pixels(std::wstring(text.begin(), text.end()), params);
    }

    ResourceID create_buffer(const std::string_view& name, const size_t size, const void* data) {
        return device->create_buffer(name, size);
    }

    ResourceID create_texture_from_data(TextureCreationParams params) {
        return device->create_texture(
            glm::ivec3(params.width, params.height, params.depth), params.type, params.format, params.data);
    }

    ResourceID create_texture2d_from_file(const std::string& path) {
        // Read image - tell stb_image we want RGBA_8
        FILE* file    = fopen(path.c_str(), "rb");
        int x         = 0;
        int y         = 0;
        stbi_uc* data = stbi_load_from_file(file, &x, &y, nullptr, 4);

        return create_texture_from_data(TextureCreationParams{
            .format = PixelFormat::RGBA_8,
            .type   = TextureType::Single2D,
            .width  = (size_t)x,
            .height = (size_t)y,
            .data   = data,
        });
    }
} // namespace Gfx
