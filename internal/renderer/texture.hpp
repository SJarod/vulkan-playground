#pragma once

#include <vulkan/vulkan.h>

class Device;
class Image;

class Texture
{
  private:
    std::weak_ptr<Device> m_device;

    uint32_t m_width;
    uint32_t m_height;

    std::unique_ptr<Image> m_image;
    VkImageView m_imageView;
    VkSampler m_sampler;

  public:
    Texture(std::weak_ptr<Device> device, uint32_t width, uint32_t height, const void *data, VkFormat format,
            VkImageTiling tiling, VkFilter samplerFilter);
    ~Texture();

    Texture(const Texture &) = delete;
    Texture &operator=(const Texture &) = delete;
    Texture(Texture &&) = delete;
    Texture &operator=(Texture &&) = delete;

  public:
    [[nodiscard]] inline const VkSampler &getSampler() const
    {
        return m_sampler;
    }
    [[nodiscard]] inline const VkImageView &getImageView() const
    {
        return m_imageView;
    }
};