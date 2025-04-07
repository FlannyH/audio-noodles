#pragma once
#include <glm/vec3.hpp>
#include <glm/matrix.hpp>

// We need this experimental header for converting quaternions to rotation matrices
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

namespace Gfx {
    struct Transform {
        glm::vec3 position{0, 0, 0};
        glm::quat rotation{1, 0, 0, 0};
        glm::vec3 scale{1, 1, 1};

        glm::mat4 as_matrix();
        glm::mat4 as_view_matrix();
        glm::vec3 forward_vector();
        glm::vec3 right_vector();
        glm::vec3 up_vector();
    };
} // namespace Gfx
