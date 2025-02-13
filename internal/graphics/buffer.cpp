#include <iostream>

#include "device.hpp"

#include "buffer.hpp"

Buffer::Buffer(std::weak_ptr<Device> device, VkDeviceSize size, VkBufferUsageFlags usage,
               VkMemoryPropertyFlags properties)
    : m_device(device), m_size(size)
{
    auto devicePtr = m_device.lock();
    auto deviceHandle = devicePtr->getHandle();

    VkBufferCreateInfo createInfo = {.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
                                     .flags = 0,
                                     .size = size,
                                     .usage = usage,
                                     .sharingMode = VK_SHARING_MODE_EXCLUSIVE};
    VkResult res = vkCreateBuffer(deviceHandle, &createInfo, nullptr, &m_handle);
    if (res != VK_SUCCESS)
    {
        std::cerr << "Failed to create buffer : " << res << std::endl;
        return;
    }

    VkMemoryRequirements memReq;
    vkGetBufferMemoryRequirements(deviceHandle, m_handle, &memReq);
    std::optional<uint32_t> memoryTypeIndex = devicePtr->findMemoryTypeIndex(memReq, properties);
    VkMemoryAllocateInfo allocInfo = {.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
                                      .allocationSize = memReq.size,
                                      .memoryTypeIndex = memoryTypeIndex.value()};
    res = vkAllocateMemory(deviceHandle, &allocInfo, nullptr, &m_memory);
    if (res != VK_SUCCESS)
    {
        std::cerr << "Failed to allocate buffer memory : " << res << std::endl;
        return;
    }

    vkBindBufferMemory(deviceHandle, m_handle, m_memory, 0);
}

void Buffer::copyDataToMemory(const void *srcData)
{
    auto deviceHandle = m_device.lock()->getHandle();
    // filling the VBO (bind and unbind CPU accessible memory)
    void *data;
    vkMapMemory(deviceHandle, m_memory, 0, m_size, 0, &data);
    // TODO : flush memory
    memcpy(data, srcData, m_size);
    // TODO : invalidate memory before reading in the pipeline
    vkUnmapMemory(deviceHandle, m_memory);
}

void Buffer::transferBufferToBuffer(VkBuffer src)
{
    auto devicePtr = m_device.lock();

    VkCommandBuffer commandBuffer = devicePtr->cmdBeginOneTimeSubmit();

    VkBufferCopy copyRegion{
        .size = m_size,
    };
    vkCmdCopyBuffer(commandBuffer, src, m_handle, 1, &copyRegion);

    devicePtr->cmdEndOneTimeSubmit(commandBuffer);
}

Buffer::~Buffer()
{
    auto deviceHandle = m_device.lock()->getHandle();

    vkFreeMemory(deviceHandle, m_memory, nullptr);

    vkDestroyBuffer(deviceHandle, m_handle, nullptr);
}