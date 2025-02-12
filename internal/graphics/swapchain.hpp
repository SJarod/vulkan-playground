#pragma once

#include <memory>
#include <vector>

#include <vulkan/vulkan.h>

class Device;
class Image;

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

    VkFormat depthFormat;
    std::unique_ptr<Image> depthImage;
    VkImageView depthImageView;

    uint32_t frameInFlightCount;

  public:
    SwapChain(const Device &device);
    ~SwapChain();

  public:
    [[nodiscard]] const VkSwapchainKHR &getHandle() const
    {
        return handle;
    }
};