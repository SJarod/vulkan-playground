#pragma once

#include <vulkan/vulkan.h>

class Device;
class Image;

class Texture
{
  private:
    const std::shared_ptr<Device> device;

  private:
    uint32_t width;
    uint32_t height;

  public:
    std::unique_ptr<Image> image;
    VkImageView imageView;
    VkSampler sampler;

  public:
    Texture(const std::shared_ptr<Device> device, uint32_t width, uint32_t height, const void *data, VkFormat format,
            VkImageTiling tiling, VkFilter samplerFilter);
    ~Texture();
};