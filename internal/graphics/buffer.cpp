#include <iostream>

#include "device.hpp"

#include "buffer.hpp"

Buffer::Buffer(const Device &device, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties)
    : device(device), size(size)
{
    VkBufferCreateInfo createInfo = {.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
                                     .flags = 0,
                                     .size = size,
                                     .usage = usage,
                                     .sharingMode = VK_SHARING_MODE_EXCLUSIVE};
    VkResult res = vkCreateBuffer(*device.handle, &createInfo, nullptr, &handle);
    if (res != VK_SUCCESS)
    {
        std::cerr << "Failed to create buffer : " << res << std::endl;
        return;
    }

    VkMemoryRequirements memReq;
    vkGetBufferMemoryRequirements(*device.handle, handle, &memReq);
    std::optional<uint32_t> memoryTypeIndex = device.findMemoryTypeIndex(memReq, properties);
    VkMemoryAllocateInfo allocInfo = {.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
                                      .allocationSize = memReq.size,
                                      .memoryTypeIndex = memoryTypeIndex.value()};
    res = vkAllocateMemory(*device.handle, &allocInfo, nullptr, &memory);
    if (res != VK_SUCCESS)
    {
        std::cerr << "Failed to allocate buffer memory : " << res << std::endl;
        return;
    }

    vkBindBufferMemory(*device.handle, handle, memory, 0);
}

void Buffer::copyDataToMemory(const void *srcData)
{
    // filling the VBO (bind and unbind CPU accessible memory)
    void *data;
    vkMapMemory(*device.handle, memory, 0, size, 0, &data);
    // TODO : flush memory
    memcpy(data, srcData, size);
    // TODO : invalidate memory before reading in the pipeline
    vkUnmapMemory(*device.handle, memory);
}

Buffer::~Buffer()
{
    vkFreeMemory(*device.handle, memory, nullptr);

    vkDestroyBuffer(*device.handle, handle, nullptr);
}