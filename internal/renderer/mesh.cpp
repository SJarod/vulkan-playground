#include <iostream>

#include "graphics/device.hpp"

#include "mesh.hpp"

Mesh::Mesh(const Device &device, const std::vector<Vertex> &vertices) : device(device), vertices(vertices)
{
    size = sizeof(Vertex) * vertices.size();

    // TODO : staging buffers for better performance (https://vulkan-tutorial.com/en/Vertex_buffers/Staging_buffer)
    VkBufferCreateInfo createInfo = {.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
                                     .flags = 0,
                                     .size = size,
                                     .usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                                     .sharingMode = VK_SHARING_MODE_EXCLUSIVE};

    VkResult res = vkCreateBuffer(*device.handle, &createInfo, nullptr, &vertexBuffer);
    if (res != VK_SUCCESS)
    {
        std::cerr << "Failed to create buffer : " << res << std::endl;
        return;
    }

    // VRAM heap
    VkMemoryRequirements memReq;
    vkGetBufferMemoryRequirements(*device.handle, vertexBuffer, &memReq);
    std::optional<uint32_t> memoryTypeIndex =
        device.findMemoryTypeIndex(memReq, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    VkMemoryAllocateInfo allocInfo = {.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
                                      .allocationSize = memReq.size,
                                      .memoryTypeIndex = memoryTypeIndex.value()};

    res = vkAllocateMemory(*device.handle, &allocInfo, nullptr, &memory);
    if (res != VK_SUCCESS)
    {
        std::cerr << "Failed to allocate memory : " << res << std::endl;
        return;
    }

    vkBindBufferMemory(*device.handle, vertexBuffer, memory, 0);

    // filling the VBO (bind and unbind CPU accessible memory)
    void *data;
    vkMapMemory(*device.handle, memory, 0, size, 0, &data);
    // TODO : flush memory
    memcpy(data, vertices.data(), size);
    // TODO : invalidate memory before reading in the pipeline
    vkUnmapMemory(*device.handle, memory);
}

Mesh::~Mesh()
{
    vkFreeMemory(*device.handle, memory, nullptr);

    vkDestroyBuffer(*device.handle, vertexBuffer, nullptr);
}