#pragma once

#include <vulkan/vulkan.h>

class Device;
class Buffer;

class Image
{
  private:
    const Device &device;

    VkFormat format;
    uint32_t width;
    uint32_t height;

    VkImageAspectFlags aspectFlags;

  private:
    VkImage handle;
    VkDeviceMemory memory;

  public:
    Image(const Device &device, VkFormat format, uint32_t width, uint32_t height, VkImageTiling tiling,
          VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImageAspectFlags aspectFlags);
    ~Image();

    void transitionImageLayout(VkImageLayout oldLayout, VkImageLayout newLayout, VkAccessFlags srcAccessMask,
                               VkAccessFlags dstAccessMask, VkPipelineStageFlags srcStageMask,
                               VkPipelineStageFlags dstStageMask);
    void copyBufferToImage(VkBuffer buffer);

    VkImageView createImageView();
};