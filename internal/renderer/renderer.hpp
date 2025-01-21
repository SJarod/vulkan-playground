#pragma once

#include <memory>

class Device;
class RenderPass;
class SwapChain;
class Pipeline;
class Mesh;

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

    int backBufferIndex = 0;
    std::vector<BackBufferT> backBuffers;

  public:
    Renderer(const Device &device, const SwapChain &swapchain, const int bufferintType = 2);
    ~Renderer();

    uint32_t acquireBackBuffer();

    void recordBackBufferPipelineCommands(uint32_t imageIndex);
    void recordBackBufferDrawObjectCommands(const Mesh &mesh);
    void recordBackBufferEnd();

    void submitBackBuffer();
    void presentBackBuffer(uint32_t imageIndex);

    void swapBuffers();
};