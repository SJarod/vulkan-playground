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

class RenderState
{
  protected:
    std::weak_ptr<Device> device;

    std::shared_ptr<Pipeline> pipeline;

    VkDescriptorPool descriptorPool;
    std::vector<VkDescriptorSet> descriptorSets;
    std::vector<std::unique_ptr<Buffer>> uniformBuffers;
    std::vector<void *> uniformBuffersMapped;

    RenderState() = default;

  public:
    virtual ~RenderState();

    virtual void updateUniformBuffers(uint32_t imageIndex, const Camera &camera);

    virtual void recordBackBufferDescriptorSetsCommands(VkCommandBuffer &commandBuffer, uint32_t imageIndex);
    virtual void recordBackBufferDrawObjectCommands(VkCommandBuffer &commandBuffer) = 0;

    [[nodiscard]] std::shared_ptr<Pipeline> getPipeline() const
    {
        return pipeline;
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
    virtual void setTexture(Texture *texture) = 0;

    virtual std::unique_ptr<RenderState> build() = 0;
};

class RenderStateDirector
{
  public:
    void createUniformAndSamplerRenderStateBuilder(RenderStateBuilderI &builder);
};

class MeshRenderState : public RenderState
{
    friend MeshRenderStateBuilder;

  public:
    std::weak_ptr<Mesh> mesh;

  public:
    void recordBackBufferDrawObjectCommands(VkCommandBuffer &commandBuffer) override;
};

class MeshRenderStateBuilder : public RenderStateBuilderI
{
  private:
    std::unique_ptr<MeshRenderState> product;

    std::weak_ptr<Device> device;

    std::vector<VkDescriptorPoolSize> poolSizes;
    uint32_t frameInFlightCount;

    Texture *texture;

  public:
    MeshRenderStateBuilder()
    {
        restart();
    }

    void restart() override
    {
        product = std::unique_ptr<MeshRenderState>(new MeshRenderState);
    }

    void setDevice(std::weak_ptr<Device> device) override
    {
        this->device = device;
        product->device = device;
    }
    void setPipeline(std::shared_ptr<Pipeline> pipeline) override;
    void addPoolSize(VkDescriptorType poolSizeType) override;
    void setFrameInFlightCount(uint32_t a) override
    {
        frameInFlightCount = a;
    }
    void setTexture(Texture *texture) override
    {
        this->texture = texture;
    }

    void setMesh(std::shared_ptr<Mesh> mesh)
    {
        product->mesh = mesh;
    }

    std::unique_ptr<RenderState> build() override;
};