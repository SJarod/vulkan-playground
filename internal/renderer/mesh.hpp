#pragma once

#include <memory>
#include <vector>
#include <vulkan/vulkan.hpp>

#include "engine/vertex.hpp"

class Device;
class Buffer;

#include "graphics/buffer.hpp"

class Mesh
{
  public:
    std::unique_ptr<Buffer> vertexBuffer;
    std::unique_ptr<Buffer> indexBuffer;

  public:
    std::vector<Vertex> vertices;
    std::vector<uint16_t> indices;

  public:
    Mesh(const Device &device, const std::vector<Vertex> &vertices, const std::vector<uint16_t> &indices);
    ~Mesh();
};
