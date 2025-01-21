#include <iostream>

#include "graphics/buffer.hpp"
#include "graphics/device.hpp"

#include "mesh.hpp"

Mesh::Mesh(const Device &device, const std::vector<Vertex> &vertices, const std::vector<uint16_t> &indices)
    : vertices(vertices), indices(indices)
{
    // vertex buffer

    {
        size_t vertexBufferSize = sizeof(Vertex) * vertices.size();

        // staging buffer

        std::unique_ptr<Buffer> stagingBuffer =
            std::make_unique<Buffer>(device, vertexBufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

        stagingBuffer->copyDataToMemory(vertices.data());

        // vertex buffer

        vertexBuffer = std::make_unique<Buffer>(device, vertexBufferSize,
                                                VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                                                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        // transfer from staging buffer to vertex buffer

        VkCommandBufferAllocateInfo allocInfo{
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .commandPool = device.commandPoolTransient,
            .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            .commandBufferCount = 1,
        };
        VkCommandBuffer commandBuffer;
        vkAllocateCommandBuffers(*device.handle, &allocInfo, &commandBuffer);
        VkCommandBufferBeginInfo beginInfo{
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
        };
        vkBeginCommandBuffer(commandBuffer, &beginInfo);
        VkBufferCopy copyRegion{
            .size = vertexBufferSize,
        };
        vkCmdCopyBuffer(commandBuffer, stagingBuffer->handle, vertexBuffer->handle, 1, &copyRegion);
        vkEndCommandBuffer(commandBuffer);
        VkSubmitInfo submitInfo{
            .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
            .commandBufferCount = 1,
            .pCommandBuffers = &commandBuffer,
        };
        vkQueueSubmit(device.graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
        vkQueueWaitIdle(device.graphicsQueue);
        vkFreeCommandBuffers(*device.handle, device.commandPool, 1, &commandBuffer);
        stagingBuffer.reset();
    }

    // index buffer

    {
        size_t indexBufferSize = sizeof(uint16_t) * indices.size();

        // staging buffer

        std::unique_ptr<Buffer> stagingBuffer =
            std::make_unique<Buffer>(device, indexBufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

        stagingBuffer->copyDataToMemory(indices.data());

        // vertex buffer

        indexBuffer = std::make_unique<Buffer>(device, indexBufferSize,
                                               VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                                               VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        // transfer from staging buffer to vertex buffer

        VkCommandBufferAllocateInfo allocInfo{
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .commandPool = device.commandPoolTransient,
            .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            .commandBufferCount = 1,
        };
        VkCommandBuffer commandBuffer;
        vkAllocateCommandBuffers(*device.handle, &allocInfo, &commandBuffer);
        VkCommandBufferBeginInfo beginInfo{
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
        };
        vkBeginCommandBuffer(commandBuffer, &beginInfo);
        VkBufferCopy copyRegion{
            .size = indexBufferSize,
        };
        vkCmdCopyBuffer(commandBuffer, stagingBuffer->handle, indexBuffer->handle, 1, &copyRegion);
        vkEndCommandBuffer(commandBuffer);
        VkSubmitInfo submitInfo{
            .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
            .commandBufferCount = 1,
            .pCommandBuffers = &commandBuffer,
        };
        vkQueueSubmit(device.graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
        vkQueueWaitIdle(device.graphicsQueue);
        vkFreeCommandBuffers(*device.handle, device.commandPool, 1, &commandBuffer);
        stagingBuffer.reset();
    }
}

Mesh::~Mesh()
{
    indexBuffer.reset();
    vertexBuffer.reset();
}
