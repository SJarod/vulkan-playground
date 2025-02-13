#pragma once

#include <vulkan/vulkan.h>

#include "graphics/image.hpp"

class Device;
class TextureBuilder;

class Texture
{
    friend TextureBuilder;

  private:
    std::weak_ptr<Device> m_device;

    uint32_t m_width;
    uint32_t m_height;

    std::unique_ptr<Image> m_image;
    VkImageView m_imageView;
    VkSampler m_sampler;

    std::vector<unsigned char> m_imageData;

    Texture() = default;

  public:
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

class TextureBuilder
{
  private:
    std::unique_ptr<Texture> m_product;

    std::weak_ptr<Device> m_device;

    VkFormat m_format;
    VkImageTiling m_tiling;
    VkFilter m_samplerFilter;

    std::string m_textureFilename;
    bool m_bLoadFromFile = false;

    void restart()
    {
        m_product = std::unique_ptr<Texture>(new Texture);
    }

  public:
    TextureBuilder()
    {
        restart();
    }

    void setDevice(std::weak_ptr<Device> device)
    {
        m_device = device;
        m_product->m_device = device;
    }

    void setWidth(uint32_t a)
    {
        m_product->m_width = a;
    }
    void setHeight(uint32_t a)
    {
        m_product->m_height = a;
    }

    void setImageData(const std::vector<unsigned char> &data)
    {
        m_product->m_imageData = data;
        m_bLoadFromFile = false;
    }

    void setTextureFilename(const std::string &filename)
    {
        m_textureFilename = filename;
        m_bLoadFromFile = true;
    }

    void setFormat(VkFormat a)
    {
        m_format = a;
    }
    void setTiling(VkImageTiling a)
    {
        m_tiling = a;
    }
    void setSamplerFilter(VkFilter a)
    {
        m_samplerFilter = a;
    }

    std::unique_ptr<Texture> build();
};

class TextureDirector
{
  public:
    void createSRGBTextureBuilder(TextureBuilder &builder);
};