#pragma once

#include <memory>

class Device;
class RenderPass;
class SwapChain;
class Pipeline;

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

    VkCommandPool commandPool;

    int backBufferIndex = 0;
    std::vector<BackBufferT> backBuffers;

  public:
    Renderer(const Device &device, const SwapChain &swapchain, const int bufferintType = 2);
    ~Renderer();

    uint32_t acquireBackBuffer();
    void recordBackBufferDrawCommands(uint32_t imageIndex, VkBuffer vertexBuffer, uint32_t vertexCount);
    void submitBackBuffer();
    void presentBackBuffer(uint32_t imageIndex);
    void swapBuffers();
};