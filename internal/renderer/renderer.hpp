#pragma once

#include <memory>

class Device;
class RenderPass;
class SwapChain;
class Pipeline;
class Mesh;
class Texture;
class Buffer;
class Camera;

struct BackBufferT
{
    VkCommandBuffer commandBuffer;

    VkSemaphore acquireSemaphore;
    VkSemaphore renderSemaphore;
    VkFence inFlightFence;
};

class MeshRenderer
{
  public:
    const Device &device;
    const RenderPass &renderPass;

    void writeDescriptorSets(const Texture &texture);

  public:
    std::unique_ptr<Pipeline> pipeline;

    VkDescriptorPool descriptorPool;
    std::vector<VkDescriptorSet> descriptorSets;
    std::vector<std::unique_ptr<Buffer>> uniformBuffers;
    std::vector<void *> uniformBuffersMapped;

    std::shared_ptr<Mesh> mesh;

    MeshRenderer(const Device &device, const RenderPass &renderPass, const std::shared_ptr<Mesh> mesh);
    ~MeshRenderer();

    void updateUniformBuffers(uint32_t imageIndex, const Camera &camera);

    void recordBackBufferPipelineCommands(VkCommandBuffer &commandBuffer, uint32_t imageIndex);
    void recordBackBufferDescriptorSetsCommands(VkCommandBuffer &commandBuffer, uint32_t imageIndex);
    void recordBackBufferDrawObjectCommands(VkCommandBuffer &commandBuffer);
    void recordBackBufferEndRenderPass(VkCommandBuffer &commandBuffer);
};

class Renderer
{
  private:
    const Device &device;
    const SwapChain &swapchain;

  public:
    int bufferingType = 2;

    std::unique_ptr<RenderPass> renderPass;

    std::vector<std::shared_ptr<MeshRenderer>> meshRenderers;

    int backBufferIndex = 0;
    std::vector<BackBufferT> backBuffers;

  public:
    Renderer(const Device &device, const SwapChain &swapchain, const int bufferintType = 2);
    ~Renderer();

    void registerRenderer(const std::shared_ptr<Mesh> mesh);

    uint32_t acquireBackBuffer();

    void recordRenderers(uint32_t imageIndex, const Camera &camera);

    void submitBackBuffer();
    void presentBackBuffer(uint32_t imageIndex);

    void swapBuffers();
};