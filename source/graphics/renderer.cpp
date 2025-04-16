#include "renderer.hpp"
#include "resource.hpp"
#include "../log.hpp"
#include "opengl/device_opengl.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <glm/geometric.hpp>

#define CIRCLE_LUT_SIZE 24

namespace Gfx {
    struct RenderInfo {
        RenderInfoType type = RenderInfoType::None;

        // Raster
        std::vector<Vertex2D> vertices_to_render;
        glm::ivec2 scissor_rect_top_left = {0, 0};
        glm::ivec2 scissor_rect_size     = {99999, 99999};
        glm::ivec2 viewport_top_left     = {0, 0};
        glm::ivec2 viewport_size         = {99999, 99999};
        ResourceID target_framebuffer    = ResourceID::invalid();
        ResourceID texture_to_bind       = ResourceID::invalid();

        // Blit
        ResourceID blit_src_texture;
        ResourceID blit_dst_texture;
        glm::ivec2 blit_src_top_left;
        glm::ivec2 blit_dst_top_left;
        glm::ivec2 blit_size;
    };

    Device* device        = nullptr; // Will be initialized to a child class of Device (e.g. DeviceOpenGL)
    glm::vec2 window_size = glm::vec2(0.0f);
    float aspect_ratio    = 1.0f;
    RenderInfo curr_render_info;
    bool render_info_dirty = false;
    std::vector<glm::ivec4> clip_rect_stack;

    // 2D rendering
    ResourceID pipeline_2d = {0, 0};
    std::vector<RenderInfo> render_queue;
    ResourceID render_queue_2d_gpu_buffer = {0, 0};
    glm::vec2 circle_lut[CIRCLE_LUT_SIZE];

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

        // Precalculate circles
        for (size_t i = 0; i < CIRCLE_LUT_SIZE; ++i) {
            float angle   = ((float)i / (float)CIRCLE_LUT_SIZE) * 2.0f * M_PI;
            circle_lut[i] = glm::vec2(cos(angle), sin(angle));
        }

        if (api == RenderAPI::OpenGL) LOG(Info, "Renderer initialized (OpenGL)");

        return true;
    }

    bool should_stay_open() { return device->should_stay_open(); }

    glm::vec2 get_window_size() { return window_size; }

    glm::vec2 get_viewport_size() { return curr_render_info.viewport_size; }

    float get_delta_time() { return device->get_delta_time(); }

    float get_fps() { return 1.0f / get_delta_time(); }

    void set_mouse_visible(bool visible) { device->set_mouse_visible(visible); }

    void set_cursor_mode(CursorMode cursor_mode) { device->set_cursor_mode(cursor_mode); }

    void set_render_target(ResourceID render_target) {
        if (curr_render_info.target_framebuffer.as_u32() != render_target.as_u32()) {
            curr_render_info.target_framebuffer = render_target;
            render_info_dirty                   = true;
        }
    }

    void begin_frame() {
        // Update window size
        int w, h;
        device->get_window_size(w, h);
        window_size.x = (float)w;
        window_size.y = (float)h;
        aspect_ratio  = get_viewport_size().x / get_viewport_size().y;

        render_queue.clear();
        device->begin_frame();
        set_render_target(ResourceID::invalid());
        set_viewport({0, 0}, {w, h});
        clip_rect_stack.clear();
        push_clip_rect({0, 0}, {w, h});
        device->clear_framebuffer(glm::vec4(0.0f, 0.0f, 0.5f, 1.0f));
    }

    void end_frame() {
        if (curr_render_info.type != RenderInfoType::None) render_queue.push_back(curr_render_info);

        if (!render_queue.empty()) {
            for (const auto& render_info: render_queue) {
                if (render_info.type == RenderInfoType::None) continue;
                if (render_info.type == RenderInfoType::Raster) {
                    if (render_info.vertices_to_render.empty()) continue;

                    device->upload_data_to_buffer(
                        render_queue_2d_gpu_buffer, 0, sizeof(Vertex2D) * render_info.vertices_to_render.size(),
                        render_info.vertices_to_render.data());
                    device->begin_raster_pass(pipeline_2d);
                    device->bind_resources({{render_queue_2d_gpu_buffer, 0}});
                    device->bind_texture(0, render_info.texture_to_bind);
                    device->set_render_target(render_info.target_framebuffer);
                    device->set_viewport(render_info.viewport_top_left, render_info.viewport_size);
                    device->set_clip_rect(render_info.scissor_rect_top_left, render_info.scissor_rect_size);
                    device->execute_raster(render_info.vertices_to_render.size());
                    device->end_raster_pass();
                    continue;
                }
                if (render_info.type == RenderInfoType::Blit) {
                    device->blit_pixels(
                        render_info.blit_src_texture, render_info.blit_dst_texture, render_info.blit_size,
                        render_info.blit_dst_top_left, render_info.blit_src_top_left);
                }
            }
        }

        device->end_frame();
    }

    void push_clip_rect(glm::ivec2 top_left, glm::ivec2 size) {
        if (curr_render_info.scissor_rect_top_left != top_left) render_info_dirty = true;
        if (curr_render_info.scissor_rect_size != size) render_info_dirty = true;
        curr_render_info.scissor_rect_top_left = top_left;
        curr_render_info.scissor_rect_size     = size;
        clip_rect_stack.push_back({top_left, size});
    }

    void pop_clip_rect() {
        clip_rect_stack.pop_back();
        if (curr_render_info.scissor_rect_top_left.x != clip_rect_stack.back().x) render_info_dirty = true;
        if (curr_render_info.scissor_rect_top_left.y != clip_rect_stack.back().y) render_info_dirty = true;
        if (curr_render_info.scissor_rect_size.x     != clip_rect_stack.back().z) render_info_dirty = true;
        if (curr_render_info.scissor_rect_size.y     != clip_rect_stack.back().w) render_info_dirty = true;
        curr_render_info.scissor_rect_top_left.x = clip_rect_stack.back().x;
        curr_render_info.scissor_rect_top_left.y = clip_rect_stack.back().y;
        curr_render_info.scissor_rect_size.x     = clip_rect_stack.back().z;
        curr_render_info.scissor_rect_size.y     = clip_rect_stack.back().w;
    }

    void set_viewport(glm::ivec2 top_left, glm::ivec2 size) {
        if (curr_render_info.viewport_top_left != top_left) render_info_dirty = true;
        if (curr_render_info.viewport_size != size) render_info_dirty = true;
        curr_render_info.viewport_top_left = top_left;
        curr_render_info.viewport_size     = size;
    }

    void draw_line_2d(glm::vec2 a, glm::vec2 b, const DrawParams& draw_params) {
        float width = draw_params.line_width;
        if (width * get_viewport_size().y < 0.5f) {
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
        if (curr_render_info.texture_to_bind.as_u32() != draw_params.texture.as_u32()) {
            curr_render_info.texture_to_bind = draw_params.texture;
            render_info_dirty                = true;
        }

        if (render_queue.empty() || render_info_dirty) {
            curr_render_info.type = RenderInfoType::Raster;
            render_queue.push_back(curr_render_info);
            render_info_dirty = false;
        }
        auto& render_info = render_queue.back();

        render_info.vertices_to_render.push_back(Vertex2D(
            Gfx::anchor_offset(v0.pos * 2.0f, draw_params.anchor_point), draw_params.depth, draw_params.color, v0.texcoord,
            draw_params.texture));
        render_info.vertices_to_render.push_back(Vertex2D(
            Gfx::anchor_offset(v1.pos * 2.0f, draw_params.anchor_point), draw_params.depth, draw_params.color, v1.texcoord,
            draw_params.texture));
        render_info.vertices_to_render.push_back(Vertex2D(
            Gfx::anchor_offset(v2.pos * 2.0f, draw_params.anchor_point), draw_params.depth, draw_params.color, v2.texcoord,
            draw_params.texture));
    }

    void draw_quad_2d(PosTexcoord v0, PosTexcoord v1, PosTexcoord v2, PosTexcoord v3, const DrawParams& draw_params) {
        draw_triangle_2d(v0, v1, v2, draw_params);
        draw_triangle_2d(v0, v2, v3, draw_params);
    }

    void draw_rectangle_2d(glm::vec2 top_left, glm::vec2 bottom_right, const DrawParams& draw_params) {
        if (draw_params.shape_outline_width <= 0.0f) {
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
            float x1 = std::min(top_left.x, bottom_right.x) + (draw_params.shape_outline_width / aspect_ratio);
            float x2 = std::max(top_left.x, bottom_right.x) - (draw_params.shape_outline_width / aspect_ratio);
            float y1 = std::min(top_left.y, bottom_right.y) + (draw_params.shape_outline_width);
            float y2 = std::max(top_left.y, bottom_right.y) - (draw_params.shape_outline_width);

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
        draw_params.line_width /= get_viewport_size().y;
        draw_line_2d(v0 / get_viewport_size(), v1 / get_viewport_size(), draw_params);
    }

    void draw_triangle_2d_pixels(PosTexcoord v0, PosTexcoord v1, PosTexcoord v2, const DrawParams& draw_params) {
        v0.pos /= get_viewport_size();
        v1.pos /= get_viewport_size();
        v2.pos /= get_viewport_size();
        draw_triangle_2d(v0, v1, v2, draw_params);
    }

    void draw_quad_2d_pixels(PosTexcoord v0, PosTexcoord v1, PosTexcoord v2, PosTexcoord v3, const DrawParams& draw_params) {
        v0.pos /= get_viewport_size();
        v1.pos /= get_viewport_size();
        v2.pos /= get_viewport_size();
        v3.pos /= get_viewport_size();
        draw_quad_2d(v0, v1, v2, v3, draw_params);
    }

    void draw_rectangle_2d_pixels(glm::vec2 top_left, glm::vec2 bottom_right, DrawParams draw_params) {
        draw_params.shape_outline_width /= get_viewport_size().y;
        draw_rectangle_2d(top_left / get_viewport_size(), bottom_right / get_viewport_size(), draw_params);
    }

    constexpr glm::vec2 anchor_offsets[] = {
        glm::vec2(0.0f, 0.0f), // TopLeft
        glm::vec2(0.5f, 0.0f), // Top
        glm::vec2(1.0f, 0.0f), // TopRight
        glm::vec2(0.0f, 0.5f), // Left
        glm::vec2(0.5f, 0.5f), // Center
        glm::vec2(1.0f, 0.5f), // Right
        glm::vec2(0.0f, 1.0f), // BottomLeft
        glm::vec2(0.5f, 1.0f), // Bottom
        glm::vec2(1.0f, 1.0f), // BottomRight
    };

    glm::vec2 anchor_offset(glm::vec2 top_left, Gfx::AnchorPoint anchor) { return top_left + anchor_offsets[(size_t)anchor]; }

    glm::vec2 anchor_offset_pixels(glm::vec2 top_left, Gfx::AnchorPoint anchor, glm::vec2 anchor_size) {
        if (anchor_size.x == 0.0f || anchor_size.y == 0.0f) anchor_size = get_viewport_size();
        return top_left + anchor_offsets[(size_t)anchor] * anchor_size;
    }

    void draw_circle_2d(glm::vec2 center, glm::vec2 size, DrawParams draw_params) {
        if (draw_params.shape_outline_width > 0.0f) {
            for (size_t i = 0; i < CIRCLE_LUT_SIZE; ++i) {
                draw_line_2d(
                    center + circle_lut[i + 0] * size, center + circle_lut[(i + 1) % CIRCLE_LUT_SIZE] * size, draw_params);
            }
        } else {
            for (size_t i = 0; i < CIRCLE_LUT_SIZE; ++i) {
                draw_triangle_2d(
                    {center + circle_lut[i + 0] * size}, {center + circle_lut[(i + 1) % CIRCLE_LUT_SIZE] * size}, {center},
                    draw_params);
            }
        }
    }

    void draw_circle_2d_pixels(glm::vec2 center, glm::vec2 size, DrawParams draw_params) {
        draw_params.shape_outline_width /= get_viewport_size().y;
        draw_circle_2d(center / get_viewport_size(), size / get_viewport_size(), draw_params);
    }

    void blit_pixels(ResourceID src, ResourceID dest, glm::ivec2 size, glm::ivec2 dest_tl, glm::ivec2 src_tl) {
        if (curr_render_info.type == RenderInfoType::Raster && !curr_render_info.vertices_to_render.empty()) {
            render_queue.push_back(curr_render_info);
        }
        curr_render_info.vertices_to_render.clear();
        curr_render_info.type              = RenderInfoType::Blit;
        curr_render_info.blit_src_texture  = src;
        curr_render_info.blit_dst_texture  = dest;
        curr_render_info.blit_src_top_left = src_tl;
        curr_render_info.blit_dst_top_left = dest_tl;
        curr_render_info.blit_size         = size;
        render_queue.push_back(curr_render_info);

        curr_render_info.type = RenderInfoType::None;
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

    float get_font_max_width() { return font->glyph_cell_size.x; }

    void draw_text_pixels(const wchar_t* text, TextDrawParams params) {
        if (!font) return;

        glm::vec2 cur_pos = params.transform.position;

        // Calculate text height, and widths for each line
        float height = static_cast<float>(font->glyph_cell_size.y) * params.transform.scale.y;
        std::vector<float> widths;
        {
            float width      = 0;
            const wchar_t* c = text - 1;
            while (*(++c) != 0) {
                if (*c == '\n') {
                    widths.push_back(width);
                    width = 0;
                    continue;
                }

                std::vector<int>& wentry = font->wchar_mapping[(size_t)*c];
                if (!wentry.empty())
                    width += static_cast<float>(font->glyph_rects[wentry[0]].size.x) * params.transform.scale.x;
            }
            widths.push_back(width); // Text may not end with a newline, so we need to push back the last width manually
        }
        height *= static_cast<float>(widths.size());

        // Calculate offsets based on text anchor point
        std::vector<glm::vec3> offsets(widths.size());
        for (size_t i = 0; i < widths.size(); i++) {
            // Apply text anchor
            glm::vec3 offset = glm::vec3(anchor_offset({0.0f, 0.0f}, params.text_anchor), 0);
            offset.x *= -widths[i];
            offset.y *= -height;

            // Add it to the offset array
            offsets[i] = offset;
        }

        int width_idx    = 0;
        const wchar_t* c = text - 1;
        while (*(++c) != 0) {
            // Handle newline
            if (*c == '\n') {
                cur_pos.x = params.transform.position.x;
                cur_pos.y += static_cast<float>(font->glyph_cell_size.y) * params.transform.scale.y;
                width_idx++;
                continue;
            }
            if (*c == '\r') {
                cur_pos.x = params.transform.position.x;
                continue;
            }
            if (*c == '\t') {
                cur_pos.x += static_cast<float>(font->glyph_cell_size.x * 4);
                continue;
            }

            // Create verts
            std::vector<int>& wentry = font->wchar_mapping[(size_t)*c];

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
        auto str = std::wstring(text.begin(), text.end());
        draw_text_pixels(str.c_str(), params);
    }

    ResourceID create_buffer(const std::string_view& name, const size_t size, const void* data) {
        return device->create_buffer(name, size);
    }

    ResourceID create_texture_from_data(TextureCreationParams params) {
        return device->create_texture(
            glm::ivec3(params.width, params.height, params.depth), params.type, params.format, params.data,
            params.is_framebuffer);
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
    void resize_texture(ResourceID id, glm::ivec3 new_resolution) { device->resize_texture(id, new_resolution); }
} // namespace Gfx
