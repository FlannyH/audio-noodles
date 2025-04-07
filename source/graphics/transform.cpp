#include "transform.hpp"

namespace Gfx {
    glm::mat4 Transform::as_matrix() {
        glm::mat4 mat_translate = glm::translate(glm::mat4(1.0f), position);
        glm::mat4 mat_rotate    = glm::toMat4(rotation);
        glm::mat4 mat_scale     = glm::scale(glm::mat4(1.0f), scale);
        return mat_translate * mat_rotate * mat_scale;
    }

    glm::mat4 Transform::as_view_matrix() { return (glm::lookAt(position, position + forward_vector(), up_vector())); }

    glm::vec3 Transform::forward_vector() { return rotation * glm::vec3{0, 0, -1}; }

    glm::vec3 Transform::right_vector() { return rotation * glm::vec3{1, 0, 0}; }

    glm::vec3 Transform::up_vector() { return rotation * glm::vec3{0, 1, 0}; }
} // namespace Gfx
