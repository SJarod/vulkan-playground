#pragma once

#include <memory>
#include <vector>

class Mesh;

class Scene
{
  private:
    std::vector<std::shared_ptr<Mesh>> m_objects;

  public:
    Scene(const std::weak_ptr<Device> device);

  public:
    [[nodiscard]] const std::vector<std::shared_ptr<Mesh>> &getObjects() const
    {
        return m_objects;
    }
};