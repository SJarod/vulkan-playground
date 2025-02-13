#pragma once

#include <glm/glm.hpp>

#include "transform.hpp"

class Camera
{
  private:
    Transform m_transform;

    float m_yFov = 45.f;
    float m_aspectRatio = 16.f / 9.f;
    float m_near = 0.1f;
    float m_far = 1000.f;

    float m_sensitivity = 0.8f;

    float m_speed = 1.f;

    bool m_bYFlip = true;

  public:
    [[nodiscard]] glm::mat4 getViewMatrix() const;
    [[nodiscard]] glm::mat4 getProjectionMatrix() const;
    [[nodiscard]] inline const float &getSensitivity() const
    {
        return m_sensitivity;
    }
    [[nodiscard]] const Transform &getTransform() const
    {
        return m_transform;
    }
    [[nodiscard]] const float &getSpeed() const
    {
        return m_speed;
    }

  public:
    void setYFlip(const bool bFlip)
    {
        m_bYFlip = bFlip;
    }
    void setTransform(const Transform &transform)
    {
        m_transform = transform;
    }
};