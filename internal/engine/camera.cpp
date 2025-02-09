#include <glm/gtc/matrix_transform.hpp>

#include "camera.hpp"

Camera::Camera()
{
    proj = glm::perspective(glm::radians(yFov), aspectRatio, near, far);
    // Y flip
    proj[1][1] *= -1;
}

glm::mat4 Camera::getViewMatrix() const
{
    glm::mat4 identity = glm::identity<glm::mat4>();

    glm::mat4 t = glm::translate(identity, transform.position);
    glm::mat4 r = glm::mat4_cast(transform.rotation);

    return r * t;
}
