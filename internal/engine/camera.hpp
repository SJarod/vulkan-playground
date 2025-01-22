#pragma once

#include <glm/glm.hpp>

#include "transform.hpp"

class Camera
{
  public:
    Transform transform;

    float yFov = 45.f;
    float aspectRatio = 16.f / 9.f;
    float near = 0.1f;
    float far = 1000.f;

    float sensitivity = 0.01f;

    glm::mat4 proj;

  public:
    Camera();

    glm::mat4 getViewMatrix() const;
};