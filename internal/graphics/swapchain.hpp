#pragma once

#include <vector>

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

    std::vector<VkImage> images;
    std::vector<VkImageView> imageViews;

  public:
    SwapChain(const Device &device);
    ~SwapChain();
};