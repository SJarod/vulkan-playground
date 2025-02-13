#pragma once

#include <cstdint>

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

class ImageBuilder;

class Image
{
    friend ImageBuilder;

  private:
    std::weak_ptr<Device> m_device;

    VkFormat m_format;
    uint32_t m_width;
    uint32_t m_height;
    uint32_t m_depth;

    VkImageAspectFlags m_aspectFlags;

    VkImage m_handle;
    VkDeviceMemory m_memory;

    Image() = default;

  public:
    ~Image();

    Image(const Image &) = delete;
    Image &operator=(const Image &) = delete;
    Image(Image &&) = delete;
    Image &operator=(Image &&) = delete;

    void transitionImageLayout(ImageLayoutTransition transition);
    void copyBufferToImage(VkBuffer buffer);

    VkImageView createImageView();

  public:
    [[nodiscard]] VkImageAspectFlags getAspectFlags() const
    {
        return m_aspectFlags;
    }

    [[nodiscard]] VkImage getHandle() const
    {
        return m_handle;
    }

    [[nodiscard]] VkFormat getFormat() const
    {
        return m_format;
    }
};

class ImageBuilder
{
  private:
    std::unique_ptr<Image> m_product;

    std::weak_ptr<Device> m_device;

    VkImageType m_imageType;

    VkImageTiling m_tiling;
    VkImageUsageFlags m_usage;
    VkMemoryPropertyFlags m_properties;

    uint32_t m_mipLevels;
    uint32_t m_arrayLayers;

    VkSampleCountFlagBits m_samples;
    VkSharingMode m_sharingMode;
    VkImageLayout m_initialLayout;

    void restart()
    {
        m_product = std::unique_ptr<Image>(new Image);
    }

  public:
    ImageBuilder()
    {
        restart();
    }

    void setDevice(std::weak_ptr<Device> a)
    {
        m_device = a;
        m_product->m_device = a;
    }
    void setImageType(VkImageType a)
    {
        m_imageType = a;
    }
    void setFormat(VkFormat a)
    {
        m_product->m_format = a;
    }
    void setWidth(uint32_t a)
    {
        m_product->m_width = a;
    }
    void setHeight(uint32_t a)
    {
        m_product->m_height = a;
    }
    void setDepth(uint32_t a)
    {
        m_product->m_depth = a;
    }
    void setTiling(VkImageTiling a)
    {
        m_tiling = a;
    }
    void setUsage(VkImageUsageFlags a)
    {
        m_usage = a;
    }
    void setProperties(VkMemoryPropertyFlags a)
    {
        m_properties = a;
    }
    void setMipLevels(uint32_t a)
    {
        m_mipLevels = a;
    }
    void setArrayLayers(uint32_t a)
    {
        m_arrayLayers = a;
    }
    void setSamples(VkSampleCountFlagBits a)
    {
        m_samples = a;
    }
    void setSharingMode(VkSharingMode a)
    {
        m_sharingMode = a;
    }
    void setInitialLayout(VkImageLayout a)
    {
        m_initialLayout = a;
    }
    void setAspectFlags(VkImageAspectFlags a)
    {
        m_product->m_aspectFlags = a;
    }

    std::unique_ptr<Image> build();
};
class ImageDirector
{
  public:
    void createImage2DBuilder(ImageBuilder &builder);
    void createDepthImage2DBuilder(ImageBuilder &builder);
    void createSampledImage2DBuilder(ImageBuilder &builder);
};

class ImageLayoutTransitionBuilder
{
  private:
    std::unique_ptr<ImageLayoutTransition> m_product;

    void restart();

  public:
    ImageLayoutTransitionBuilder()
    {
        restart();
    }

    void setSrcAccessMask(VkAccessFlags a) const
    {
        m_product->barrier.srcAccessMask = a;
    }
    void setDstAccessMask(VkAccessFlags a) const
    {
        m_product->barrier.dstAccessMask = a;
    }
    void setOldLayout(VkImageLayout a) const
    {
        m_product->barrier.oldLayout = a;
    }
    void setNewLayout(VkImageLayout a) const
    {
        m_product->barrier.newLayout = a;
    }
    void setSrcQueueFamilyIndex(uint32_t a) const
    {
        m_product->barrier.srcQueueFamilyIndex = a;
    }
    void setDstQueueFamilyIndex(uint32_t a) const
    {
        m_product->barrier.dstQueueFamilyIndex = a;
    }
    void setImage(Image &a) const
    {
        m_product->barrier.image = a.getHandle();
        m_product->barrier.subresourceRange.aspectMask = a.getAspectFlags();
    }
    void setBaseMipLevel(uint32_t a) const
    {
        m_product->barrier.subresourceRange.baseMipLevel = a;
    }
    void setLevelCount(uint32_t a) const
    {
        m_product->barrier.subresourceRange.levelCount = a;
    }
    void setBaseArrayLayer(uint32_t a) const
    {
        m_product->barrier.subresourceRange.baseArrayLayer = a;
    }
    void setLayerCount(uint32_t a) const
    {
        m_product->barrier.subresourceRange.layerCount = a;
    }
    void setSrcStageMask(VkPipelineStageFlags a)
    {
        m_product->srcStageMask = a;
    }
    void setDstStageMask(VkPipelineStageFlags a)
    {
        m_product->dstStageMask = a;
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
