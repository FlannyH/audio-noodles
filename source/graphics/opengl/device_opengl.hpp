#pragma once
#include "../device.hpp"

#include <map>
#include <deque>

struct GLFWwindow;

namespace Gfx {
    struct Transform;

    class DeviceOpenGL : public Device {
      public:
        // Initialization
        DeviceOpenGL(int width, int height, const char* window_title);
        void handle_resize(int width, int height) override;
        void get_window_size(int& width, int& height) override;

        // Common rendering
        bool should_stay_open() override;
        void set_full_screen(bool full_screen) override;
        void begin_frame() override;
        void end_frame() override;
        void clear_framebuffer(glm::vec4 color);
        void blit_pixels(ResourceID src, ResourceID dest, glm::ivec2 size, glm::ivec2 dest_tl, glm::ivec2 src_tl) override;
        void set_camera(const Transform& transform) override;
        void set_clip_rect(glm::ivec2 top_left, glm::ivec2 size) override;
        void set_viewport(glm::ivec2 top_left, glm::ivec2 size) override;
        void set_render_target(ResourceID render_target) override;
        float get_delta_time() override;

        // Rendering pipelines
        ResourceID load_pipeline_raster(const char* shader_vs, const char* shader_ps) override;
        void begin_raster_pass(const ResourceID raster_pipeline) override;
        void end_raster_pass() override;
        void execute_raster(const size_t n_vertices, const int vertex_offset) override;
        void set_constants(const std::vector<uint32_t>& constants) override;

        // Resource management
        const Resource* const get_resource(const ResourceID id);
        void delete_resource(const ResourceID resource_to_destroy) override;
        void bind_resources(const std::vector<ResourceWithOffset>& bindings) override;
        ResourceID create_buffer(const std::string_view& name, const size_t size_bytes, const void* data = nullptr) override;
        void upload_data_to_buffer(
            const ResourceID buffer, const size_t offset_bytes, const size_t size_bytes, const void* data) override;
        ResourceID create_texture(
            const glm::ivec3 resolution, const TextureType type, const PixelFormat format, const void* data,
            const bool is_framebuffer) override;
        void bind_texture(int slot, const ResourceID texture) override;
        void resize_texture(ResourceID id, glm::ivec3 new_resolution) override;

        // Input
        void input_setup() override;
        void set_mouse_visible(bool visible) override;
        void set_cursor_mode(Gfx::CursorMode cursor_mode) override;

      private:
        PairResourceID allocate_resource_slot(const ResourceType type);

        GLFWwindow* window = nullptr;
        std::vector<Resource*> resources;
        std::vector<uint64_t> bindless_texture_handles;
        std::deque<uint32_t> recycled_resource_slots;
        uint32_t empty_vao;
        double delta_time = 0.0;
        std::vector<GLFWcursor*> cursors;
        ResourceID active_framebuffer = ResourceID::invalid();

        ResourceID render_pass_active = ResourceID::invalid();
    };
} // namespace Gfx
