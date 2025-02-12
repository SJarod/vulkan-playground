#pragma once

#include <vulkan/vulkan.h>

class Device;
class Buffer;
class ImageLayoutTransitionBuilder;

class ImageLayoutTransition
{
    friend ImageLayoutTransitionBuilder;

  private:
    ImageLayoutTransition() = default;

  public:
    VkImageMemoryBarrier barrier = {};
    VkPipelineStageFlags srcStageMask = {};
    VkPipelineStageFlags dstStageMask = {};
};

class Image
{
  private:
    const Device &device;

    VkFormat format;
    uint32_t width;
    uint32_t height;

  public:
    VkImageAspectFlags aspectFlags;

    VkImage handle;
    VkDeviceMemory memory;

  public:
    Image(const Device &device, VkFormat format, uint32_t width, uint32_t height, VkImageTiling tiling,
          VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImageAspectFlags aspectFlags);
    ~Image();

    void transitionImageLayout(ImageLayoutTransition transition);
    void copyBufferToImage(VkBuffer buffer);

    VkImageView createImageView();
};

class ImageLayoutTransitionBuilder
{
  private:
    std::unique_ptr<ImageLayoutTransition> product;

    void restart();

  public:
    ImageLayoutTransitionBuilder()
    {
        restart();
    }

    void setSrcAccessMask(VkAccessFlags a) const
    {
        product->barrier.srcAccessMask = a;
    }
    void setDstAccessMask(VkAccessFlags a) const
    {
        product->barrier.dstAccessMask = a;
    }
    void setOldLayout(VkImageLayout a) const
    {
        product->barrier.oldLayout = a;
    }
    void setNewLayout(VkImageLayout a) const
    {
        product->barrier.newLayout = a;
    }
    void setSrcQueueFamilyIndex(uint32_t a) const
    {
        product->barrier.srcQueueFamilyIndex = a;
    }
    void setDstQueueFamilyIndex(uint32_t a) const
    {
        product->barrier.dstQueueFamilyIndex = a;
    }
    void setImage(Image &a) const
    {
        product->barrier.image = a.handle;
        product->barrier.subresourceRange.aspectMask = a.aspectFlags;
    }
    void setBaseMipLevel(uint32_t a) const
    {
        product->barrier.subresourceRange.baseMipLevel = a;
    }
    void setLevelCount(uint32_t a) const
    {
        product->barrier.subresourceRange.levelCount = a;
    }
    void setBaseArrayLayer(uint32_t a) const
    {
        product->barrier.subresourceRange.baseArrayLayer = a;
    }
    void setLayerCount(uint32_t a) const
    {
        product->barrier.subresourceRange.layerCount = a;
    }
    void setSrcStageMask(VkPipelineStageFlags a)
    {
        product->srcStageMask = a;
    }
    void setDstStageMask(VkPipelineStageFlags a)
    {
        product->dstStageMask = a;
    }

    std::unique_ptr<ImageLayoutTransition> build();
};

class ImageLayoutTransitionDirector
{
  public:
    template <VkImageLayout TFrom, VkImageLayout TTo> void createBuilder(ImageLayoutTransitionBuilder &builder) const
    {
        builder.setOldLayout(TFrom);
        builder.setNewLayout(TTo);
    }

    template <>
    void createBuilder<VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL>(
        ImageLayoutTransitionBuilder &builder) const
    {
        builder.setOldLayout(VK_IMAGE_LAYOUT_UNDEFINED);
        builder.setNewLayout(VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
        builder.setSrcAccessMask(0);
        builder.setDstAccessMask(VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
                                 VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT);
        builder.setSrcStageMask(VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT);
        builder.setDstStageMask(VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT);
    }

    template <>
    void createBuilder<VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL>(
        ImageLayoutTransitionBuilder &builder) const
    {
        builder.setOldLayout(VK_IMAGE_LAYOUT_UNDEFINED);
        builder.setNewLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
        builder.setSrcAccessMask(0);
        builder.setDstAccessMask(VK_ACCESS_TRANSFER_WRITE_BIT);
        builder.setSrcStageMask(VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT);
        builder.setDstStageMask(VK_PIPELINE_STAGE_TRANSFER_BIT);
    }

    template <>
    void createBuilder<VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL>(
        ImageLayoutTransitionBuilder &builder) const
    {
        builder.setOldLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
        builder.setNewLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        builder.setSrcAccessMask(VK_ACCESS_TRANSFER_WRITE_BIT);
        builder.setDstAccessMask(VK_ACCESS_SHADER_READ_BIT);
        builder.setSrcStageMask(VK_PIPELINE_STAGE_TRANSFER_BIT);
        builder.setDstStageMask(VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
    }
};
