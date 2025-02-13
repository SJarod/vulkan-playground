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
    const std::weak_ptr<Device> m_device;

    std::unique_ptr<Buffer> m_vertexBuffer;
    std::unique_ptr<Buffer> m_indexBuffer;

    std::vector<Vertex> m_vertices;
    std::vector<uint16_t> m_indices;

    std::unique_ptr<Texture> m_texture;

    void createVertexBuffer();
    void createIndexBuffer();
    void setVerticesFromAiScene(const aiScene *pScene);
    void setIndicesFromAiScene(const aiScene *pScene);

  public:
    Mesh(const std::weak_ptr<Device> device, const std::vector<Vertex> &vertices, const std::vector<uint16_t> &indices);
    Mesh(const std::weak_ptr<Device> device, const char *modelFilename, const char *textureFilename);
    ~Mesh();

    Mesh(const Mesh &) = delete;
    Mesh &operator=(const Mesh &) = delete;
    Mesh(Mesh &&) = delete;
    Mesh &operator=(Mesh &&) = delete;

  public:
    [[nodiscard]] inline const VkBuffer getVertexBufferHandle() const
    {
        return m_vertexBuffer->getHandle();
    }
    [[nodiscard]] inline const VkBuffer getIndexBufferHandle() const
    {
        return m_indexBuffer->getHandle();
    }
    [[nodiscard]] inline const uint32_t getVertexCount() const
    {
        return m_vertices.size();
    }
    [[nodiscard]] inline const uint32_t getIndexCount() const
    {
        return m_indices.size();
    }
    [[nodiscard]] inline const Texture *getTexture() const
    {
        return m_texture.get();
    }

  public:
    void setTexture(std::unique_ptr<Texture> texture)
    {
        m_texture = std::move(texture);
    }
};
