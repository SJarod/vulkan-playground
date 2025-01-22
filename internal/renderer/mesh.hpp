#pragma once

#include <memory>
#include <vector>
#include <vulkan/vulkan.hpp>

#include "engine/vertex.hpp"
#include "graphics/buffer.hpp"

class Device;
class Buffer;
class aiScene;

class Mesh
{
  private:
    const Device &device;

  private:
    void initVerticesAndIndices();

  public:
    std::unique_ptr<Buffer> vertexBuffer;
    std::unique_ptr<Buffer> indexBuffer;

  public:
    std::vector<Vertex> vertices;
    std::vector<uint16_t> indices;

  public:
    Mesh(const Device &device, const std::vector<Vertex> &vertices, const std::vector<uint16_t> &indices);
    Mesh(const Device &device, const aiScene *pScene);
    ~Mesh();
};
