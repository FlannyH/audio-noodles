#define GLFW_INCLUDE_NONE
#include <glbinding/glbinding.h>
#include <glbinding/gl/gl.h>
#include <GLFW/glfw3.h>
#include <cassert>
#include <iostream>
#include <fstream>
#include <string>
#include <memory>
#include "../../common.hpp"
#include "../../input.hpp"
#include "../../log.hpp"
#include "device_opengl.hpp"

namespace Gfx {
    static void glfw_error_callback(int error, const char* description) { LOG(Error, description); }

    static void opengl_debug_callback(
        gl::GLenum source, gl::GLenum type, gl::GLuint id, gl::GLenum severity, gl::GLsizei length, const gl::GLchar* message,
        const gl::GLvoid* userParam) {
        (void)length;
        (void)userParam;
        if (id == 131218) // http://stackoverflow.com/questions/12004396/opengl-debug-context-performance-warning
            return;

        char const* source_str;
        char const* type_str;
        char const* severity_str;

        switch (source) {
        case gl::GL_DEBUG_SOURCE_API: source_str = "API"; break;
        case gl::GL_DEBUG_SOURCE_APPLICATION: source_str = "Application"; break;
        case gl::GL_DEBUG_SOURCE_WINDOW_SYSTEM: source_str = "Window System"; break;
        case gl::GL_DEBUG_SOURCE_SHADER_COMPILER: source_str = "Shader Compiler"; break;
        case gl::GL_DEBUG_SOURCE_THIRD_PARTY: source_str = "Third Party"; break;
        case gl::GL_DEBUG_SOURCE_OTHER: source_str = "Other"; break;
        default: source_str = "Unknown"; break;
        }

        switch (type) {
        case gl::GL_DEBUG_TYPE_ERROR: type_str = "Error"; break;
        case gl::GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: type_str = "Deprecated Behavior"; break;
        case gl::GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR: type_str = "Undefined Behavior"; break;
        case gl::GL_DEBUG_TYPE_PORTABILITY_ARB: type_str = "Portability"; break;
        case gl::GL_DEBUG_TYPE_PERFORMANCE: type_str = "Performance"; break;
        case gl::GL_DEBUG_TYPE_OTHER: type_str = "Other"; break;
        default: type_str = "Unknown"; break;
        }

        Log::Level log_level = Log::Level::Info;

        switch (severity) {
        case gl::GL_DEBUG_SEVERITY_HIGH:
            severity_str = "High";
            log_level    = Log::Level::Error;
            break;
        case gl::GL_DEBUG_SEVERITY_MEDIUM:
            severity_str = "Medium";
            log_level    = Log::Level::Warning;
            break;
        case gl::GL_DEBUG_SEVERITY_LOW:
            severity_str = "Low";
            log_level    = Log::Level::Info;
            break;
        default: severity_str = "Unknown"; return;
        }

        Log::write(
            log_level,
            "GL Debug Callback:\n source: %i:%s \n type: %i:%s \n id: %i \n "
            "severity: %i:%s \n    message: %s",
            source, source_str, type, type_str, id, severity, severity_str, message);
    }

    bool load_shader_part(const char* path, const ShaderType type, const gl::GLuint* program) {
        const gl::GLenum shader_types[5] = {
            gl::GL_INVALID_ENUM, gl::GL_VERTEX_SHADER, gl::GL_FRAGMENT_SHADER, gl::GL_GEOMETRY_SHADER, gl::GL_COMPUTE_SHADER,
        };

        // Read shader source file
        size_t shader_size = 0;
        char* shader_data  = NULL;

        // Open file
        std::ifstream file(path, std::ifstream::binary);
        if (!file.is_open()) {
            // Log error
            LOG(Error, "Shader source file \"%s\" not found!", path);
            return false;
        }

        // Get size
        file.seekg(0, file.end);
        shader_size = file.tellg();
        file.seekg(0, file.beg);

        // Read data
        shader_data              = new char[shader_size + 1];
        shader_data[shader_size] = 0;
        file.read(shader_data, shader_size);

        // Create shader on GPU
        const gl::GLenum type_to_create = shader_types[(int)type];
        const gl::GLuint shader         = gl::glCreateShader(type_to_create);

        // Compile shader source
        const gl::GLint shader_size_gl = (gl::GLint)shader_size;
        gl::glShaderSource(shader, 1, (const gl::GLchar* const*)&shader_data, &shader_size_gl);
        gl::glCompileShader(shader);

        // Error checking
        gl::GLboolean result = gl::GL_FALSE;
        int log_length;
        gl::glGetShaderiv(shader, gl::GL_COMPILE_STATUS, &result);
        gl::glGetShaderiv(shader, gl::GL_INFO_LOG_LENGTH, &log_length);
        std::unique_ptr<char[]> frag_shader_error = std::make_unique<char[]>(log_length);
        gl::glGetShaderInfoLog(shader, log_length, NULL, &frag_shader_error[0]);
        if (log_length > 0) {
            // Log error
            LOG(Error, "File '%s':\n\n%s\n", path, &frag_shader_error[0]);
            return false;
        }

        // Attach to program
        gl::glAttachShader(*program, shader);

        return true;
    }

    DeviceOpenGL::DeviceOpenGL(int width, int height, const char* window_title) {
        if (!glfwInit()) {
            LOG(Fatal, "Failed to initialize GLFW");
            exit(1);
        }

        glfwSetErrorCallback(glfw_error_callback);

        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        window = glfwCreateWindow(width, height, window_title, NULL, NULL);
        if (!window) {
            LOG(Fatal, "Failed to create GLFW window");
            glfwTerminate();
            exit(1);
        }

        glfwMakeContextCurrent(window);
        glbinding::initialize(glfwGetProcAddress);
        gl::glEnable(gl::GL_DEBUG_OUTPUT);
        gl::glEnable(gl::GL_DEBUG_OUTPUT_SYNCHRONOUS);
        gl::glDebugMessageCallback(opengl_debug_callback, NULL);

        gl::glGenVertexArrays(1, &empty_vao);

        input_setup();
    }

    void DeviceOpenGL::handle_resize(int width, int height) { gl::glViewport(0, 0, width, height); }

    void DeviceOpenGL::get_window_size(int& width, int& height) { glfwGetFramebufferSize(window, &width, &height); }

    bool DeviceOpenGL::should_stay_open() { return !glfwWindowShouldClose(window); }

    void DeviceOpenGL::set_full_screen(bool full_screen) { TODO(); }

    void DeviceOpenGL::begin_frame() {
        // Calculate delta time
        static double prev_time = 0.0;
        static double curr_time = 0.0;
        prev_time               = curr_time;
        curr_time               = glfwGetTime();
        delta_time              = curr_time - prev_time;

        // Any frametime above 1.0 seconds (or less than 1 fps) is spicy, so we should limit the deltatime
        if (delta_time > 1.0) {
            delta_time = 1.0;
        }

        glfwPollEvents();
    }

    void DeviceOpenGL::end_frame() { glfwSwapBuffers(window); }

    void DeviceOpenGL::clear_framebuffer(glm::vec4 color) {
        gl::glDisable(gl::GL_SCISSOR_TEST);
        gl::glClearColor(color.r, color.g, color.b, color.a);
        gl::glClear(gl::ClearBufferMask::GL_COLOR_BUFFER_BIT);
        gl::glEnable(gl::GL_SCISSOR_TEST);
    }

    void DeviceOpenGL::set_camera(const Transform& transform) { TODO(); }

    void DeviceOpenGL::set_clip_rect(glm::vec2 top_left, glm::vec2 size) {
        int w, h;
        this->get_window_size(w, h);
        const gl::GLint left   = (gl::GLint)top_left.x;
        const gl::GLint top    = h - (gl::GLint)top_left.y - size.y;
        const gl::GLint width  = (gl::GLint)size.x;
        const gl::GLint height = (gl::GLint)size.y;
        gl::glScissor(left, top, width, height);
    }

    float DeviceOpenGL::get_delta_time() { return (float)delta_time; }

    ResourceID DeviceOpenGL::load_pipeline_raster(const char* shader_vs, const char* shader_ps) {
        // Create program
        const gl::GLuint shader_gpu = gl::glCreateProgram();

        // Load shader parts
        const bool vert_loaded = load_shader_part(shader_vs, ShaderType::Vertex, &shader_gpu);
        const bool frag_loaded = load_shader_part(shader_ps, ShaderType::Pixel, &shader_gpu);

        // Make sure it worked
        if (vert_loaded == false) LOG(Error, "Failed to load shader \"%s\"!", shader_vs);
        if (frag_loaded == false) LOG(Error, "Failed to load shader \"%s\"!", shader_ps);

        // Link
        gl::glLinkProgram(shader_gpu);
        int success;
        gl::glGetProgramiv(shader_gpu, gl::GL_LINK_STATUS, &success);
        if (!success) {
            char info_log[512] = {0};
            int length         = 0;
            gl::glGetProgramInfoLog(shader_gpu, 512, NULL, info_log);
            LOG(Error, "Failed to link program%s%s", (length > 0) ? ": " : "", (length > 0) ? info_log : "");
        }

        // Keep track of its resource ID
        auto resource_id_pair                = allocate_resource_slot(ResourceType::Pipeline);
        PipelineResource* resource           = new PipelineResource();
        resource->base.gpu_handle32          = shader_gpu;
        resources.at(resource_id_pair.id.id) = (Resource*)resource;

        return resource_id_pair.id;
    }

    void DeviceOpenGL::begin_raster_pass(const ResourceID raster_pipeline) {
        render_pass_active = raster_pipeline;
        auto pipeline      = resources.at(raster_pipeline.id); // no need to cast, only accessing variables from base class
        gl::glUseProgram(pipeline->gpu_handle32);
        gl::glBindVertexArray(empty_vao);
        gl::glEnable(gl::GL_SCISSOR_TEST);
        gl::glUniform1i(0, 0); // texture_bound = 0
    }

    void DeviceOpenGL::end_raster_pass() {
        render_pass_active = ResourceID::invalid();
        gl::glUseProgram(0);
    }

    void DeviceOpenGL::execute_raster(const size_t n_vertices, const int vertex_offset) {
        gl::glDrawArrays(gl::GLenum::GL_TRIANGLES, vertex_offset, (gl::GLsizei)n_vertices);
    }

    void DeviceOpenGL::set_constants(const std::vector<uint32_t>& constants) {
        int i = 0;
        for (const uint32_t constant: constants) gl::glUniform1ui(i++, constant);
    }

    const Resource* const DeviceOpenGL::get_resource(const ResourceID id) {
        return (const Resource* const)(&resources.at(id.id));
    }

    void DeviceOpenGL::delete_resource(const ResourceID resource_to_destroy) {
        const auto type = (ResourceType)resource_to_destroy.type;
        if (type == ResourceType::Buffer) {
            auto buffer = (BufferResource*)(resources.at(resource_to_destroy.id));
            gl::glDeleteBuffers(1, &buffer->base.gpu_handle32);
            delete buffer;
        } else {
            TODO();
        }

        resources.at(resource_to_destroy.id) = nullptr;
    }

    void DeviceOpenGL::bind_resources(const std::vector<ResourceWithOffset>& bindings) {
        assert(render_pass_active.is_valid() && "No render pass active!");

        gl::GLuint index = 0;
        for (const auto& [resource, offset]: bindings) {
            const auto type = (ResourceType)resource.type;
            if (type == ResourceType::Buffer) {
                auto buffer = (BufferResource*)(resources.at(resource.id));
                gl::glBindBufferBase(gl::GL_SHADER_STORAGE_BUFFER, index, buffer->base.gpu_handle32);
            } else {
                TODO();
                break;
            }
            index++;
        }
    }

    ResourceID DeviceOpenGL::create_buffer(const std::string_view& name, const size_t size_bytes, const void* data) {
        assert(size_bytes > 0 && "Attempt to create GPU buffer with size 0!");

        // Create buffer object
        gl::GLuint gl_id = 0;
        gl::glGenBuffers(1, &gl_id);

        // Keep track of the resource
        auto resource_id_pair                = allocate_resource_slot(ResourceType::Buffer);
        BufferResource* resource             = new BufferResource();
        resource->base.gpu_handle32          = gl_id;
        resource->size                       = size_bytes;
        resources.at(resource_id_pair.id.id) = (Resource*)resource;

        // todo(lily): support user specified buffer flags
        const auto flags = gl::BufferStorageMask::GL_DYNAMIC_STORAGE_BIT;

        // Allocate buffer on GPU
        gl::glBindBuffer(gl::GLenum::GL_SHADER_STORAGE_BUFFER, gl_id);
        gl::glBufferData(gl::GLenum::GL_SHADER_STORAGE_BUFFER, size_bytes, data, gl::GLenum::GL_DYNAMIC_DRAW);
        gl::glObjectLabel(gl::GLenum::GL_BUFFER, gl_id, (gl::GLsizei)name.size(), name.data());

        return resource_id_pair.id;
    }

    void DeviceOpenGL::upload_data_to_buffer(
        const ResourceID buffer, const size_t offset_bytes, const size_t size_bytes, const void* data) {
        auto buffer_resource = (BufferResource*)(resources.at(buffer.id));
        if ((offset_bytes + size_bytes) > buffer_resource->size) {
            LOG(Warning, "GPU buffer data upload overflow! Data will not be copied.");
            return;
        }
        gl::glNamedBufferSubData(buffer_resource->base.gpu_handle32, offset_bytes, size_bytes, data);
    }

    gl::GLenum texture_type_to_gl(const TextureType type) {
        switch (type) {
        case TextureType::Single2D: return gl::GL_TEXTURE_2D;
        case TextureType::Array2D: return gl::GL_TEXTURE_2D_ARRAY;
        case TextureType::Single3D: return gl::GL_TEXTURE_3D;
        default: return gl::GL_INVALID_ENUM;
        }
    }

    gl::GLenum texture_format_to_gl_format(const PixelFormat format) {
        switch (format) {
        case PixelFormat::R_8: return gl::GL_R;
        case PixelFormat::RG_8: return gl::GL_RG;
        case PixelFormat::RGB_8: return gl::GL_RGB;
        case PixelFormat::RGBA_8: return gl::GL_RGBA;
        default: return gl::GL_INVALID_ENUM;
        }
    }

    gl::GLenum texture_format_to_gl_data_type(const PixelFormat format) {
        switch (format) {
        case PixelFormat::R_8: return gl::GL_UNSIGNED_BYTE;
        case PixelFormat::RG_8: return gl::GL_UNSIGNED_BYTE;
        case PixelFormat::RGB_8: return gl::GL_UNSIGNED_BYTE;
        case PixelFormat::RGBA_8: return gl::GL_UNSIGNED_BYTE;
        default: return gl::GL_INVALID_ENUM;
        }
    }

    /*
        When `type` is equal to `TextureType::Single2D`, `resolution.z` is ignored.
        When `type` is equal to `TextureType::Array2D`, `resolution.z` is the length of the array (number of layers).
        When `type` is equal to `TextureType::Single3D`, `resolution.z` is what you would expect: the depth or Z
       resolution `data` may be null, in which case the texture is created but no data is uploaded to it.
    */
    ResourceID DeviceOpenGL::create_texture(
        const glm::ivec3 resolution, const TextureType type, const PixelFormat format, const void* data) {
        assert((resolution.x > 0 && resolution.y > 0 && resolution.z > 0) && "Attempt to create GPU texture with size 0!");
        assert(type != TextureType::Invalid && "Attempt to create GPU texture with invalid type!");
        assert(format != PixelFormat::Invalid && "Attempt to create GPU texture with invalid format!");

        const gl::GLenum gl_type      = texture_type_to_gl(type);
        const gl::GLenum gl_format    = texture_format_to_gl_format(format);
        const gl::GLenum gl_data_type = texture_format_to_gl_data_type(format);

        gl::GLuint gl_id = 0;
        gl::glGenTextures(1, &gl_id);
        gl::glBindTexture(gl_type, gl_id);

        switch (type) {
        case TextureType::Single2D:
            gl::glTexImage2D(gl_type, 0, gl_format, resolution.x, resolution.y, 0, gl_format, gl_data_type, data);
            break;
        case TextureType::Single3D: // Same as Array2D, so fall through
        case TextureType::Array2D:
            gl::glTexImage3D(gl_type, 0, gl_format, resolution.x, resolution.y, resolution.z, 0, gl_format, gl_data_type, data);
            break;
        }

        gl::glTexImage2D(gl_type, 0, gl_format, resolution.x, resolution.y, 0, gl_format, gl_data_type, data);
        gl::glGenerateMipmap(gl_type);
        gl::glTexParameteri(
            gl_type, gl::GL_TEXTURE_MIN_FILTER, gl::GL_NEAREST); // todo(lily): expose this to the caller in some way
        gl::glTexParameteri(
            gl_type, gl::GL_TEXTURE_MAG_FILTER,
            gl::GL_NEAREST); //             especially useful for a future material system
        gl::glTexParameteri(gl_type, gl::GL_TEXTURE_WRAP_S, gl::GL_REPEAT);
        gl::glTexParameteri(gl_type, gl::GL_TEXTURE_WRAP_T, gl::GL_REPEAT);
        gl::glBindTexture(gl_type, 0);

        // Keep track of its resource ID
        auto resource_id_pair                = allocate_resource_slot(ResourceType::Texture);
        TextureResource* resource            = new TextureResource();
        resource->base.gpu_handle32          = gl_id;
        resource->width                      = resolution.x;
        resource->height                     = resolution.y;
        resource->pixel_format               = format;
        resources.at(resource_id_pair.id.id) = (Resource*)resource;

        return resource_id_pair.id;
    }

    void DeviceOpenGL::bind_texture(int slot, const ResourceID texture) {
        if (texture.is_valid() && texture.type == (uint32_t)ResourceType::Texture) {
            gl::glBindTextureUnit(slot, resources.at(texture.id)->gpu_handle32);
            gl::glUniform1i(0, 1); // texture_bound = 1
        } else {
            gl::glBindTextureUnit(slot, 0);
            gl::glUniform1i(0, 0); // texture_bound = 0
        }
    }

    Input::Key glfw_to_key(int key) {
        // A to Z
        if (key >= GLFW_KEY_A && key <= GLFW_KEY_Z) {
            return (Input::Key)(key - GLFW_KEY_A + (int)Input::Key::A);
        }
        // 0 to 9
        if (key >= GLFW_KEY_0 && key <= GLFW_KEY_9) {
            return (Input::Key)(key - GLFW_KEY_0 + (int)Input::Key::_0);
        }
        // F1 to F12
        if (key >= GLFW_KEY_F1 && key <= GLFW_KEY_F12) {
            return (Input::Key)(key - GLFW_KEY_F1 + (int)Input::Key::F1);
        }
        // Misc
        if (key == GLFW_KEY_SPACE) return Input::Key::Space;
        if (key == GLFW_KEY_ESCAPE) return Input::Key::Escape;
        if (key == GLFW_KEY_ENTER) return Input::Key::Enter;
        if (key == GLFW_KEY_TAB) return Input::Key::Tab;
        if (key == GLFW_KEY_LEFT_SHIFT) return Input::Key::LeftShift;
        if (key == GLFW_KEY_LEFT_CONTROL) return Input::Key::LeftControl;
        if (key == GLFW_KEY_LEFT_ALT) return Input::Key::LeftAlt;
        if (key == GLFW_KEY_RIGHT_SHIFT) return Input::Key::RightShift;
        if (key == GLFW_KEY_RIGHT_CONTROL) return Input::Key::RightControl;
        if (key == GLFW_KEY_RIGHT_ALT) return Input::Key::RightAlt;
        if (key == GLFW_KEY_UP) return Input::Key::Up;
        if (key == GLFW_KEY_DOWN) return Input::Key::Down;
        if (key == GLFW_KEY_LEFT) return Input::Key::Left;
        if (key == GLFW_KEY_RIGHT) return Input::Key::Right;
        return Input::Key::Invalid;
    }

    static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
        Input::InputData* incoming_input_data = (Input::InputData*)glfwGetWindowUserPointer(window);
        switch (action) {
        case GLFW_PRESS: incoming_input_data->keys[(size_t)glfw_to_key(key)] = true; break;
        case GLFW_RELEASE: incoming_input_data->keys[(size_t)glfw_to_key(key)] = false; break;
        default: break;
        }
    }

    static void cursor_callback(GLFWwindow* window, double xpos, double ypos) {
        Input::InputData* incoming_input_data = (Input::InputData*)glfwGetWindowUserPointer(window);
        incoming_input_data->mouse_x          = xpos;
        incoming_input_data->mouse_y          = ypos;
    }

    static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
        Input::InputData* incoming_input_data = (Input::InputData*)glfwGetWindowUserPointer(window);
        Input::MouseButton mouse_button       = Input::MouseButton::Left;
        switch (button) {
        case GLFW_MOUSE_BUTTON_LEFT: mouse_button = Input::MouseButton::Left; break;
        case GLFW_MOUSE_BUTTON_RIGHT: mouse_button = Input::MouseButton::Right; break;
        case GLFW_MOUSE_BUTTON_MIDDLE: mouse_button = Input::MouseButton::Middle; break;
        }

        switch (action) {
        case GLFW_PRESS: incoming_input_data->mouse_buttons[(size_t)mouse_button] = true; break;
        case GLFW_RELEASE: incoming_input_data->mouse_buttons[(size_t)mouse_button] = false; break;
        }
    }

    static void mouse_scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
        Input::InputData* incoming_input_data = (Input::InputData*)glfwGetWindowUserPointer(window);
        incoming_input_data->mouse_scroll_x += xoffset;
        incoming_input_data->mouse_scroll_y += yoffset;
    }

    void DeviceOpenGL::input_setup() {
        glfwSetWindowUserPointer(window, Device::fetch_incoming_input_data_pointer());
        glfwSetKeyCallback(window, key_callback);
        glfwSetCursorPosCallback(window, cursor_callback);
        glfwSetMouseButtonCallback(window, mouse_button_callback);
        glfwSetScrollCallback(window, mouse_scroll_callback);
    }

    void DeviceOpenGL::set_mouse_visible(bool visible) {
        glfwSetInputMode(window, GLFW_CURSOR, visible ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_HIDDEN);
    }

    PairResourceID DeviceOpenGL::allocate_resource_slot(const ResourceType type) {
        // Reuse previously deallocated slots
        if (recycled_resource_slots.empty() == false) {
            const ResourceID slot = {.id = recycled_resource_slots.front(), .type = (uint32_t)type};
            recycled_resource_slots.pop_front();
            return PairResourceID{
                .resource = resources.at(slot.id),
                .id       = slot,
            };
        }

        // If there aren't any, allocate a new one
        const ResourceID slot = {.id = (uint32_t)resources.size(), .type = (uint32_t)type};
        resources.push_back(nullptr);
        return PairResourceID{
            .resource = resources.at(slot.id),
            .id       = slot,
        };
    }
} // namespace Gfx
