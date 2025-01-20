#pragma once

#include <memory>

class Device;
class RenderPass;
class SwapChain;
class Pipeline;

class Renderer
{
  private:
    const Device &device;
    const SwapChain &swapchain;

  public:
    int bufferingCount = 2;

    std::unique_ptr<RenderPass> renderPass;

    std::unique_ptr<Pipeline> pipeline;

    VkCommandPool commandPool;
    std::vector<VkCommandBuffer> commandBuffers;

    std::vector<VkSemaphore> drawSemaphores;
    std::vector<VkSemaphore> presentSemaphores;
    std::vector<VkFence> inFlightFences;

  public:
    Renderer(const Device &device, const SwapChain &swapchain, const int bufferingCount = 2);
    ~Renderer();
};