#pragma once

#include <memory>
#include <vector>
#include <vulkan/vulkan.hpp>

#include "engine/vertex.hpp"
#include "graphics/buffer.hpp"

class Device;
class Buffer;
class Texture;
class aiScene;

class Mesh
{
  private:
    const std::shared_ptr<Device> device;

  private:
    void createVertexBuffer();
    void createIndexBuffer();
    void setVerticesFromAiScene(const aiScene *pScene);
    void setIndicesFromAiScene(const aiScene *pScene);

  public:
    std::unique_ptr<Buffer> vertexBuffer;
    std::unique_ptr<Buffer> indexBuffer;

  public:
    std::vector<Vertex> vertices;
    std::vector<uint16_t> indices;

    std::unique_ptr<Texture> texture;

  public:
    Mesh(const std::shared_ptr<Device> device, const std::vector<Vertex> &vertices,
         const std::vector<uint16_t> &indices);
    Mesh(const std::shared_ptr<Device> device, const char *modelFilename, const char *textureFilename);
    ~Mesh();
};
