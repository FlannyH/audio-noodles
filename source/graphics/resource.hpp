#pragma once
#include <cstdint>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

namespace Gfx {
    enum class ResourceType {
        Invalid = 0,
        Texture,
        Buffer,
        Pipeline,
    };

    struct ResourceID {
        uint32_t id   : 24;
        uint32_t type : 8;
        uint32_t as_u32() const { return (id & 0xFFFFFF) | (type << 24); }
        static ResourceID from_u32(uint32_t in) { return ResourceID{.id = in & 0x00FFFFFF, .type = in >> 24}; }
        bool is_valid() const { return (ResourceType)type != ResourceType::Invalid; }
        static ResourceID invalid() { return {0, (uint32_t)ResourceType::Invalid}; }
    };

    enum class TextureType {
        Invalid = 0,
        Single2D,
        Array2D,
        Single3D,
    };

    // Implement support for this in the renderer
    enum class TextureScaleMode { Stretch, Tile, Slice };

    enum class PixelFormat {
        Invalid = 0,
        R_8,
        RG_8,
        RGB_8,
        RGBA_8,
    };

    enum class ShaderType { Invalid = 0, Vertex, Pixel, Geometry, Compute };

    struct Vertex2D {
        glm::vec4 position; // vec4 for padding
        glm::vec4 color;
        glm::vec2 texcoords;
        ResourceID texture_id;
        uint32_t padding;
        Vertex2D(
            glm::vec3 _pos, glm::vec4 _color, glm::vec2 _texcoords = glm::vec2(0.0f),
            ResourceID _texture_id = ResourceID::invalid()) {
            position   = glm::vec4(_pos, 0.0f);
            color      = _color;
            texcoords  = _texcoords;
            texture_id = _texture_id;
        }
        Vertex2D(
            glm::vec2 _pos, float depth, glm::vec4 _color, glm::vec2 _texcoords = glm::vec2(0.0f),
            ResourceID _texture_id = ResourceID::invalid()) {
            position   = glm::vec4(_pos, depth, 0.0f);
            color      = _color;
            texcoords  = _texcoords;
            texture_id = _texture_id;
        }
    };

    // Should be added to every resource struct. I initially use inheritance for this, but I
    // need to guarantee the memory layout for the way I'm storing and accessing these structs.
    struct Resource {
        // OpenGL prefers 32-bit integers, but the other APIs may use 64 bit integers or pointers, so let's support both
        union {
            uint64_t gpu_handle64; // Can contain any 64 bit integer or pointer used by a rendering API to identify GPU
                                   // resources.
            uint32_t gpu_handle32; // Can contain any 32 bit integer used by a rendering API to identify GPU resources.
        };
    };

    struct PairResourceID {
        Resource* resource;
        ResourceID id;
    };

    struct TextureResource {
        Resource base;
        uint32_t width, height;
        PixelFormat pixel_format;
    };

    struct BufferResource {
        Resource base;
        uint64_t size;
    };

    struct PipelineResource {
        Resource base;
    };

    enum class CursorMode { Arrow = 0, IBeam, Crosshair, Hand, ResizeEW, ResizeNS, ResizeNWSE, ResizeNESW, Denied };
} // namespace Gfx
