#pragma once

#include <memory>
#include <vector>

#include <vulkan/vulkan.h>

class Pipeline;
class Device;
class Buffer;
class Camera;
class Mesh;
class MeshRenderStateBuilder;

class UniformBlock
{
  class MVP
  {
    public:
      glm::mat4 model;
      glm::mat4 view;
      glm::mat4 proj;
  };

  // TODO : move uniform block in RenderPhase class (or in pipeline ?)

  struct LightT
  {
      glm::vec3 color;
      float intensity;
  };

  struct DirectionalLight : public LightT
  {
      glm::vec3 direction;
  };

  struct PointLightT : public LightT
  {
      glm::vec3 position;
      glm::vec3 attenuation;
  };

  VkDescriptorPool m_descriptorPool;
  std::vector<VkDescriptorSet> m_descriptorSets;
  std::vector<std::unique_ptr<Buffer>> m_uniformBuffers;
  std::vector<void *> m_uniformBuffersMapped;
};

class RenderStateABC
{
  protected:

    std::weak_ptr<Device> m_device;

    std::shared_ptr<Pipeline> m_pipeline;

    std::unique_ptr<UniformBlock> m_uniformBlock;

    RenderStateABC() = default;

  public:
    virtual ~RenderStateABC();

    virtual void updateUniformBuffers(uint32_t imageIndex, const Camera &camera);

    virtual void recordBackBufferDescriptorSetsCommands(VkCommandBuffer &commandBuffer, uint32_t imageIndex);
    virtual void recordBackBufferDrawObjectCommands(VkCommandBuffer &commandBuffer) = 0;

  public:
    [[nodiscard]] std::shared_ptr<Pipeline> getPipeline() const
    {
        return m_pipeline;
    }
};

class RenderStateBuilderI
{
  public:
    virtual void restart() = 0;

    virtual void setDevice(std::weak_ptr<Device> device) = 0;
    virtual void setPipeline(std::shared_ptr<Pipeline> pipeline) = 0;
    virtual void addPoolSize(VkDescriptorType poolSizeType) = 0;
    virtual void setFrameInFlightCount(uint32_t a) = 0;
    virtual void setTexture(std::weak_ptr<Texture> texture) = 0;

    virtual std::unique_ptr<RenderStateABC> build() = 0;
};

class MeshRenderState : public RenderStateABC
{
    friend MeshRenderStateBuilder;

  private:
    std::weak_ptr<Mesh> m_mesh;

  public:
    void recordBackBufferDrawObjectCommands(VkCommandBuffer &commandBuffer) override;
};

class MeshRenderStateBuilder : public RenderStateBuilderI
{
  private:
    std::unique_ptr<MeshRenderState> m_product;

    std::weak_ptr<Device> m_device;

    std::vector<VkDescriptorPoolSize> m_poolSizes;
    uint32_t m_frameInFlightCount;

    std::weak_ptr<Texture> m_texture;

  public:
    MeshRenderStateBuilder()
    {
        restart();
    }

    void restart() override
    {
        m_product = std::unique_ptr<MeshRenderState>(new MeshRenderState);
    }

    void setDevice(std::weak_ptr<Device> device) override
    {
        m_device = device;
        m_product->m_device = device;
    }
    void setPipeline(std::shared_ptr<Pipeline> pipeline) override;
    void addPoolSize(VkDescriptorType poolSizeType) override;
    void setFrameInFlightCount(uint32_t a) override
    {
        m_frameInFlightCount = a;
    }
    void setTexture(std::weak_ptr<Texture> texture) override
    {
        m_texture = texture;
    }

    void setMesh(std::shared_ptr<Mesh> mesh)
    {
        m_product->m_mesh = mesh;
    }

    std::unique_ptr<RenderStateABC> build() override;
};
