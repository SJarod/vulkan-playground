#pragma once

#include <vector>

#include <vulkan/vulkan.h>

class Device;
class SwapChain;

class RenderPass
{
  public:
    const Device &device;
    const SwapChain &swapchain;

  public:
    VkRenderPass handle;
    std::vector<VkFramebuffer> framebuffers;

  public:
    RenderPass(const Device &device, const SwapChain &swapchain);
    ~RenderPass();
};