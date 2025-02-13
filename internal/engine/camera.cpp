#include <glm/gtc/matrix_transform.hpp>

#include "camera.hpp"

glm::mat4 Camera::getViewMatrix() const
{
    glm::mat4 identity = glm::identity<glm::mat4>();

    glm::mat4 t = glm::translate(identity, m_transform.position);
    glm::mat4 r = glm::mat4_cast(m_transform.rotation);

    return r * t;
}

glm::mat4 Camera::getProjectionMatrix() const
{
    glm::mat4 proj = glm::perspective(glm::radians(m_yFov), m_aspectRatio, m_near, m_far);
    if (m_bYFlip)
        proj[1][1] *= -1;
    return proj;
}
