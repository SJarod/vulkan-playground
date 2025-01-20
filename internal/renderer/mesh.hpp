#pragma once

#include <vector>
#include <vulkan/vulkan.hpp>

#include "engine/vertex.hpp"

class Device;

class Mesh
{
  private:
    const Device &device;

    size_t size;

  public:
    VkBuffer vertexBuffer;
    VkDeviceMemory memory;

  private:
    std::vector<Vertex> vertices;

  public:
    Mesh(const Device &device, const std::vector<Vertex>& vertices);
    ~Mesh();
};
