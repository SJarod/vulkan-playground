#include <iostream>

#include "graphics/buffer.hpp"
#include "graphics/device.hpp"
#include "graphics/image.hpp"

#include "texture.hpp"

Texture::Texture(const std::shared_ptr<Device> device, uint32_t width, uint32_t height, const void *data,
                 VkFormat format, VkImageTiling tiling, VkFilter samplerFilter)
    : device(device), width(width), height(height)
{
    size_t imageSize = width * height * 4;

    Buffer stagingBuffer = Buffer(*device, imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                  VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    stagingBuffer.copyDataToMemory(data);

    image = std::make_unique<Image>(*device, format, width, height, tiling,
                                    VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                                    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    image->transitionImageLayout(VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 0,
                                 VK_ACCESS_TRANSFER_WRITE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                                 VK_PIPELINE_STAGE_TRANSFER_BIT);
    image->copyBufferToImage(stagingBuffer.handle);
    image->transitionImageLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                 VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT,
                                 VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);

    // image view

    imageView = image->createImageView();

    // sampler

    VkSamplerCreateInfo createInfo = {
        .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
        .magFilter = samplerFilter,
        .minFilter = samplerFilter,
        .mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
        .addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT,
        .addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT,
        .addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT,
        .mipLodBias = 0.f,
        .anisotropyEnable = device->features.samplerAnisotropy,
        .maxAnisotropy = device->props.limits.maxSamplerAnisotropy,
        .compareEnable = VK_FALSE,
        .compareOp = VK_COMPARE_OP_ALWAYS,
        .minLod = 0.f,
        .maxLod = 0.f,
        .borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
        .unnormalizedCoordinates = VK_FALSE,
    };

    VkResult res = vkCreateSampler(*device->handle, &createInfo, nullptr, &sampler);
    if (res != VK_SUCCESS)
        std::cerr << "Failed to create image sampler : " << res << std::endl;
}

Texture::~Texture()
{
    vkDestroySampler(*device->handle, sampler, nullptr);
    vkDestroyImageView(*device->handle, imageView, nullptr);
    image.reset();
}