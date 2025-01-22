#pragma once

#include <memory>

class Device;
class RenderPass;
class SwapChain;
class Pipeline;
class Mesh;
class Buffer;

struct BackBufferT
{
    VkCommandBuffer commandBuffer;

    VkSemaphore acquireSemaphore;
    VkSemaphore renderSemaphore;
    VkFence inFlightFence;
};

class Renderer
{
  private:
    const Device &device;
    const SwapChain &swapchain;

  public:
    int bufferingType = 2;

    std::unique_ptr<RenderPass> renderPass;

    std::unique_ptr<Pipeline> pipeline;

    VkDescriptorPool descriptorPool;
    std::vector<VkDescriptorSet> descriptorSets;
    std::vector<std::unique_ptr<Buffer>> uniformBuffers;
    std::vector<void *> uniformBuffersMapped;

    int backBufferIndex = 0;
    std::vector<BackBufferT> backBuffers;

  public:
    Renderer(const Device &device, const SwapChain &swapchain, const int bufferintType = 2);
    ~Renderer();

    void writeDescriptorSets();

    void updateUniformBuffers(uint32_t imageIndex);

    uint32_t acquireBackBuffer();

    void recordBackBufferBeginRenderPass(uint32_t imageIndex);
    void recordBackBufferDescriptorSetsCommands(uint32_t imageIndex);
    void recordBackBufferDrawObjectCommands(const Mesh &mesh);
    void recordBackBufferEndRenderPass();

    void submitBackBuffer();
    void presentBackBuffer(uint32_t imageIndex);

    void swapBuffers();
};