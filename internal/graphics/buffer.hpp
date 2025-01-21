#pragma once

#include <vulkan/vulkan.hpp>

class Device;

class Buffer
{
  private:
    const Device &device;

  public:
    VkBuffer handle;
    VkDeviceMemory memory;
    size_t size;

  public:
    Buffer(const Device &device, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties);
    ~Buffer();

    void copyDataToMemory(const void *srcData);
};