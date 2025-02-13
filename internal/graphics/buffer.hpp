#pragma once

#include <memory>

#include <vulkan/vulkan.hpp>

class Device;

class Buffer
{
  private:
    std::weak_ptr<Device> m_device;

    VkBuffer m_handle;
    VkDeviceMemory m_memory;
    size_t m_size;

  public:
    Buffer(std::weak_ptr<Device> device, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties);
    ~Buffer();

    void copyDataToMemory(const void *srcData);

    void transferBufferToBuffer(VkBuffer src);

  public:
    [[nodiscard]] inline const VkBuffer &getHandle() const
    {
        return m_handle;
    }

    [[nodiscard]] inline const VkDeviceMemory &getMemory() const
    {
        return m_memory;
    }
};