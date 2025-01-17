#pragma once

#include <vulkan/vulkan.h>

class Device;

class SwapChain
{
  public:
    const Device &device;

  public:
    VkSwapchainKHR handle;

    VkFormat imageFormat;
    VkExtent2D extent;

  public:
    SwapChain(const Device &device);
    ~SwapChain();
};