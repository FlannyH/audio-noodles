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
