#include <iostream>

#include "graphics/buffer.hpp"
#include "graphics/device.hpp"
#include "graphics/image.hpp"

#include "texture.hpp"

Texture::Texture(std::weak_ptr<Device> device, uint32_t width, uint32_t height, const void *data, VkFormat format,
                 VkImageTiling tiling, VkFilter samplerFilter)
    : m_device(device), m_width(width), m_height(height)
{
    size_t imageSize = width * height * 4;

    std::unique_ptr<Buffer> stagingBuffer =
        std::make_unique<Buffer>(device, imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    stagingBuffer->copyDataToMemory(data);

    m_image = std::make_unique<Image>(device, format, width, height, tiling,
                                      VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                                      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_IMAGE_ASPECT_COLOR_BIT);

    ImageLayoutTransitionBuilder iltb;
    ImageLayoutTransitionDirector iltd;

    iltd.createBuilder<VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL>(iltb);
    iltb.setImage(*m_image);
    m_image->transitionImageLayout(*iltb.build());

    m_image->copyBufferToImage(stagingBuffer->getHandle());

    iltd.createBuilder<VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL>(iltb);
    iltb.setImage(*m_image);
    m_image->transitionImageLayout(*iltb.build());

    // image view

    m_imageView = m_image->createImageView();

    // sampler

    auto devicePtr = m_device.lock();
    auto deviceHandle = devicePtr->getHandle();

    VkSamplerCreateInfo createInfo = {
        .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
        .magFilter = samplerFilter,
        .minFilter = samplerFilter,
        .mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
        .addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT,
        .addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT,
        .addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT,
        .mipLodBias = 0.f,
        .anisotropyEnable = devicePtr->getPhysicalDeviceFeatures().samplerAnisotropy,
        .maxAnisotropy = devicePtr->getPhysicalDeviceProperties().limits.maxSamplerAnisotropy,
        .compareEnable = VK_FALSE,
        .compareOp = VK_COMPARE_OP_ALWAYS,
        .minLod = 0.f,
        .maxLod = 0.f,
        .borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
        .unnormalizedCoordinates = VK_FALSE,
    };

    VkResult res = vkCreateSampler(deviceHandle, &createInfo, nullptr, &m_sampler);
    if (res != VK_SUCCESS)
        std::cerr << "Failed to create image sampler : " << res << std::endl;
}

Texture::~Texture()
{
    auto deviceHandle = m_device.lock()->getHandle();
    vkDestroySampler(deviceHandle, m_sampler, nullptr);
    vkDestroyImageView(deviceHandle, m_imageView, nullptr);
    m_image.reset();
}