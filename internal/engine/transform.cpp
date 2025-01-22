#include <glm/gtc/matrix_transform.hpp>

#include "transform.hpp"

glm::mat4 Transform::getTransformMatrix() const
{
    glm::mat4 identity = glm::identity<glm::mat4>();

    glm::mat4 t = glm::translate(identity, position);
    glm::mat4 r = glm::mat4_cast(rotation);
    glm::mat4 s = glm::scale(identity, scale);

    return t * r * s;
}