#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

class Transform
{
  public:
    glm::vec3 position = glm::vec3(0.f);
    glm::quat rotation = glm::identity<glm::quat>();
    glm::vec3 scale = glm::vec3(1.f);

  public:
    glm::mat4 getTransformMatrix() const;
};