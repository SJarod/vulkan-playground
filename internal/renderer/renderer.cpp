#include <iostream>

#include "graphics/device.hpp"
#include "graphics/pipeline.hpp"
#include "graphics/renderpass.hpp"
#include "graphics/swapchain.hpp"

#include "renderer.hpp"

Renderer::Renderer(const Device &device, const SwapChain &swapchain, const int bufferingCount)
    : device(device), swapchain(swapchain), bufferingCount(bufferingCount)
{
    renderPass = std::make_unique<RenderPass>(device, swapchain);

    pipeline = std::make_unique<Pipeline>(device, "triangle", *renderPass, swapchain.extent);

    VkCommandPoolCreateInfo createInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .queueFamilyIndex = device.graphicsFamilyIndex.value(),
    };

    VkResult res = vkCreateCommandPool(*device.handle, &createInfo, nullptr, &commandPool);
    if (res != VK_SUCCESS)
    {
        std::cerr << "Failed to create command pool : " << res << std::endl;
        return;
    }

    VkCommandBufferAllocateInfo allocInfo = {.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
                                             .commandPool = commandPool,
                                             .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
                                             .commandBufferCount = static_cast<uint32_t>(bufferingCount)};

    std::vector<VkCommandBuffer> commandBuffer(bufferingCount);
    res = vkAllocateCommandBuffers(*device.handle, &allocInfo, commandBuffer.data());
    if (res != VK_SUCCESS)
    {
        std::cerr << "Failed to allocate command buffers : " << res << std::endl;
        return;
    }

    // synchronization

    drawSemaphores.resize(bufferingCount);
    presentSemaphores.resize(bufferingCount);
    inFlightFences.resize(bufferingCount);

    VkSemaphoreCreateInfo semaphoreCreateInfo = {.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
    VkFenceCreateInfo fenceCreateInfo = {.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
                                         .flags = VK_FENCE_CREATE_SIGNALED_BIT};
    for (int i = 0; i < bufferingCount; ++i)
    {
        VkSemaphore &drawSemaphore = drawSemaphores[i];
        res = vkCreateSemaphore(*device.handle, &semaphoreCreateInfo, nullptr, &drawSemaphore);
        if (res != VK_SUCCESS)
        {
            std::cerr << "Failed to create semaphore : " << res << std::endl;
            return;
        }

        VkSemaphore &presentSemaphore = presentSemaphores[i];
        res = vkCreateSemaphore(*device.handle, &semaphoreCreateInfo, nullptr, &presentSemaphore);
        if (res != VK_SUCCESS)
        {
            std::cerr << "Failed to create semaphore : " << res << std::endl;
            return;
        }

        VkFence &inFlightFence = inFlightFences[i];
        res = vkCreateFence(*device.handle, &fenceCreateInfo, nullptr, &inFlightFence);
        if (res != VK_SUCCESS)
        {
            std::cerr << "Failed to create fence : " << res << std::endl;
            return;
        }
    }
}

Renderer::~Renderer()
{
    for (int i = 0; i < bufferingCount; ++i)
    {
        vkDestroyFence(*device.handle, inFlightFences[i], nullptr);
        vkDestroySemaphore(*device.handle, presentSemaphores[i], nullptr);
        vkDestroySemaphore(*device.handle, drawSemaphores[i], nullptr);
    }
    vkDestroyCommandPool(*device.handle, commandPool, nullptr);

    pipeline.reset();
    renderPass.reset();
}
