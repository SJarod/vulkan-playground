#include <iostream>

#include "buffer.hpp"
#include "device.hpp"

#include "image.hpp"

Image::Image(const Device &device, VkFormat format, uint32_t width, uint32_t height, VkImageTiling tiling,
             VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImageAspectFlags aspectFlags)
    : device(device), format(format), width(width), height(height), aspectFlags(aspectFlags)
{
    VkImageCreateInfo createInfo = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .flags = 0,
        .imageType = VK_IMAGE_TYPE_2D,
        .format = format,
        .extent =
            {
                .width = width,
                .height = height,
                .depth = 1U,
            },
        .mipLevels = 1U,
        .arrayLayers = 1U,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .tiling = tiling,
        .usage = usage,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
    };

    VkResult res = vkCreateImage(*device.handle, &createInfo, nullptr, &handle);
    if (res != VK_SUCCESS)
    {
        std::cerr << "Failed to create image : " << res << std::endl;
        return;
    }

    VkMemoryRequirements memReq;
    vkGetImageMemoryRequirements(*device.handle, handle, &memReq);
    std::optional<uint32_t> memoryTypeIndex = device.findMemoryTypeIndex(memReq, properties);

    VkMemoryAllocateInfo allocInfo = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .allocationSize = memReq.size,
        .memoryTypeIndex = memoryTypeIndex.value(),
    };

    res = vkAllocateMemory(*device.handle, &allocInfo, nullptr, &memory);
    if (res != VK_SUCCESS)
    {
        std::cerr << "Failed to allocate memory : " << res << std::endl;
        return;
    }

    vkBindImageMemory(*device.handle, handle, memory, 0);
}

Image::~Image()
{
    vkFreeMemory(*device.handle, memory, nullptr);
    vkDestroyImage(*device.handle, handle, nullptr);
}

void Image::transitionImageLayout(VkImageLayout oldLayout, VkImageLayout newLayout, VkAccessFlags srcAccessMask,
                                  VkAccessFlags dstAccessMask, VkPipelineStageFlags srcStageMask,
                                  VkPipelineStageFlags dstStageMask)
{
    VkCommandBuffer commandBuffer = device.cmdBeginOneTimeSubmit();

    VkImageMemoryBarrier barrier = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        .srcAccessMask = srcAccessMask,
        .dstAccessMask = dstAccessMask,
        .oldLayout = oldLayout,
        .newLayout = newLayout,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .image = handle,
        .subresourceRange =
            {
                .aspectMask = aspectFlags,
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = 1,
            },
    };
    vkCmdPipelineBarrier(commandBuffer, srcStageMask, dstStageMask, 0, 0, nullptr, 0, nullptr, 1, &barrier);

    device.cmdEndOneTimeSubmit(commandBuffer);
}

void Image::copyBufferToImage(VkBuffer buffer)
{
    VkCommandBuffer commandBuffer = device.cmdBeginOneTimeSubmit();

    VkBufferImageCopy region = {
        .bufferOffset = 0,
        .bufferRowLength = 0,
        .bufferImageHeight = 0,
        .imageSubresource =
            {
                .aspectMask = aspectFlags,
                .mipLevel = 0,
                .baseArrayLayer = 0,
                .layerCount = 1,
            },
        .imageOffset =
            {
                .x = 0,
                .y = 0,
                .z = 0,
            },
        .imageExtent =
            {
                .width = width,
                .height = height,
                .depth = 1,
            },
    };

    vkCmdCopyBufferToImage(commandBuffer, buffer, handle, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

    device.cmdEndOneTimeSubmit(commandBuffer);
}

VkImageView Image::createImageView()
{
    VkImageViewCreateInfo createInfo = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .image = handle,
        .viewType = VK_IMAGE_VIEW_TYPE_2D,
        .format = format,
        .components =
            {
                .r = VK_COMPONENT_SWIZZLE_R,
                .g = VK_COMPONENT_SWIZZLE_G,
                .b = VK_COMPONENT_SWIZZLE_B,
                .a = VK_COMPONENT_SWIZZLE_A,
            },
        .subresourceRange =
            {
                .aspectMask = aspectFlags,
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = 1,
            },
    };

    VkImageView imageView;
    VkResult res = vkCreateImageView(*device.handle, &createInfo, nullptr, &imageView);
    if (res != VK_SUCCESS)
        std::cerr << "Failed to create image view : " << res << std::endl;

    return imageView;
}
