#pragma once
#include "resource.hpp"
#include <string_view>
#include "../input.hpp"

namespace Gfx {
    struct Transform;

    struct ResourceWithOffset {
        ResourceID id;
        uint32_t offset;
    };

    class Device {
      public:
        // Initialization
        Device(){};
        virtual void handle_resize(int width, int height) = 0;
        virtual void get_window_size(int& width, int& height) = 0;

        // Common rendering
        virtual bool should_stay_open() = 0;
        virtual void set_full_screen(bool full_screen) = 0;
        virtual void begin_frame() = 0;
        virtual void end_frame() = 0;
        virtual void clear_framebuffer(glm::vec4 color) = 0;
        virtual void set_camera(const Transform& transform) = 0;
        virtual float get_delta_time() = 0;

        // Rendering pipelines
        virtual ResourceID load_pipeline_raster(const char* shader_vs, const char* shader_ps) = 0;
        virtual void begin_raster_pass(const ResourceID raster_pipeline) = 0;
        virtual void end_raster_pass() = 0;
        virtual void execute_raster(const size_t n_vertices, const int vertex_offset = 0) = 0;
        virtual void set_constants(const std::vector<uint32_t>& constants) = 0;

        // Resource management
        virtual const Resource* const get_resource(const ResourceID id) = 0;
        virtual void delete_resource(const ResourceID resource_to_destroy) = 0;
        virtual void bind_resources(const std::vector<ResourceWithOffset>& bindings) = 0;
        virtual ResourceID
        create_buffer(const std::string_view& name, const size_t size_bytes, const void* data = nullptr) = 0;
        virtual void upload_data_to_buffer(
            const ResourceID buffer, const size_t offset_bytes, const size_t size_bytes, const void* data) = 0;
        virtual ResourceID create_texture(
            const glm::ivec3 resolution, const TextureType type, const PixelFormat format, const void* data) = 0;
        virtual void bind_texture(int slot, const ResourceID texture) = 0;

        // Input
        virtual void input_setup() = 0;
        Input::InputData* fetch_incoming_input_data_pointer();
    };
} // namespace Gfx
