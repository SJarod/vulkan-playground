#pragma once

#include <vector>

#include <vulkan/vulkan.h>

class Device;
class SwapChain;

class RenderPass
{
  private:
    const Device &device;
    const SwapChain &swapchain;

  private:
    VkRenderPass handle;
    std::vector<VkFramebuffer> framebuffers;

  public:
    RenderPass(const Device &device, const SwapChain &swapchain);
    ~RenderPass();
};