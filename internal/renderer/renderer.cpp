#include <iostream>

#include "graphics/device.hpp"
#include "graphics/pipeline.hpp"
#include "graphics/renderpass.hpp"
#include "graphics/swapchain.hpp"

#include "mesh.hpp"

#include "renderer.hpp"

Renderer::Renderer(const Device &device, const SwapChain &swapchain, const int bufferingType)
    : device(device), swapchain(swapchain), bufferingType(bufferingType)
{
    renderPass = std::make_unique<RenderPass>(device, swapchain);

    pipeline = std::make_unique<Pipeline>(device, "triangle", *renderPass, swapchain.extent);

    // back buffers

    backBuffers.resize(bufferingType);

    VkCommandBufferAllocateInfo allocInfo = {.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
                                             .commandPool = device.commandPool,
                                             .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
                                             .commandBufferCount = 1U};
    for (int i = 0; i < bufferingType; ++i)
    {
        VkResult res = vkAllocateCommandBuffers(*device.handle, &allocInfo, &backBuffers[i].commandBuffer);
        if (res != VK_SUCCESS)
        {
            std::cerr << "Failed to allocate command buffers : " << res << std::endl;
            return;
        }
    }

    // synchronization

    VkSemaphoreCreateInfo semaphoreCreateInfo = {.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
    VkFenceCreateInfo fenceCreateInfo = {.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
                                         .flags = VK_FENCE_CREATE_SIGNALED_BIT};
    for (int i = 0; i < bufferingType; ++i)
    {
        VkResult res =
            vkCreateSemaphore(*device.handle, &semaphoreCreateInfo, nullptr, &backBuffers[i].acquireSemaphore);
        if (res != VK_SUCCESS)
        {
            std::cerr << "Failed to create semaphore : " << res << std::endl;
            return;
        }

        res = vkCreateSemaphore(*device.handle, &semaphoreCreateInfo, nullptr, &backBuffers[i].renderSemaphore);
        if (res != VK_SUCCESS)
        {
            std::cerr << "Failed to create semaphore : " << res << std::endl;
            return;
        }

        res = vkCreateFence(*device.handle, &fenceCreateInfo, nullptr, &backBuffers[i].inFlightFence);
        if (res != VK_SUCCESS)
        {
            std::cerr << "Failed to create fence : " << res << std::endl;
            return;
        }
    }
}

Renderer::~Renderer()
{
    for (int i = 0; i < bufferingType; ++i)
    {
        vkDestroyFence(*device.handle, backBuffers[i].inFlightFence, nullptr);
        vkDestroySemaphore(*device.handle, backBuffers[i].renderSemaphore, nullptr);
        vkDestroySemaphore(*device.handle, backBuffers[i].acquireSemaphore, nullptr);
    }

    pipeline.reset();
    renderPass.reset();
}

uint32_t Renderer::acquireBackBuffer()
{
    uint32_t imageIndex;
    VkResult res = vkAcquireNextImageKHR(*device.handle, swapchain.handle, UINT64_MAX,
                                         backBuffers[backBufferIndex].acquireSemaphore, VK_NULL_HANDLE, &imageIndex);
    if (res != VK_SUCCESS)
    {
        std::cerr << "Failed to acquire next image : " << res << std::endl;
        return -1;
    }

    vkWaitForFences(*device.handle, 1, &backBuffers[backBufferIndex].inFlightFence, VK_TRUE, UINT64_MAX);
    vkResetFences(*device.handle, 1, &backBuffers[backBufferIndex].inFlightFence);

    return imageIndex;
}

void Renderer::recordBackBufferPipelineCommands(uint32_t imageIndex)
{
    VkCommandBuffer &commandBuffer = backBuffers[backBufferIndex].commandBuffer;

    vkResetCommandBuffer(commandBuffer, 0);

    VkCommandBufferBeginInfo commandBufferBeginInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO, .flags = 0, .pInheritanceInfo = nullptr};
    VkResult res = vkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo);
    if (res != VK_SUCCESS)
    {
        std::cerr << "Failed to begin recording command buffer : " << res << std::endl;
        return;
    }

    VkClearValue clearColor = {.color = {0.2f, 0.2f, 0.2f, 1.f}};
    VkRenderPassBeginInfo renderPassBeginInfo = {.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
                                                 .renderPass = renderPass->handle,
                                                 .framebuffer = renderPass->framebuffers[imageIndex],
                                                 .renderArea = {.offset = {0, 0}, .extent = swapchain.extent},
                                                 .clearValueCount = 1,
                                                 .pClearValues = &clearColor};
    vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->handle);

    VkViewport viewport = {.x = 0.f,
                           .y = 0.f,
                           .width = static_cast<float>(swapchain.extent.width),
                           .height = static_cast<float>(swapchain.extent.height),
                           .minDepth = 0.f,
                           .maxDepth = 1.f};
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
    VkRect2D scissor = {.offset = {0, 0}, .extent = swapchain.extent};
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
}
void Renderer::recordBackBufferDrawObjectCommands(const Mesh &mesh)
{
    VkCommandBuffer &commandBuffer = backBuffers[backBufferIndex].commandBuffer;
    VkBuffer vbos[] = {mesh.vertexBuffer->handle};
    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, vbos, offsets);
    vkCmdBindIndexBuffer(commandBuffer, mesh.indexBuffer->handle, 0, VK_INDEX_TYPE_UINT16);
    vkCmdDrawIndexed(commandBuffer, mesh.indices.size(), 1, 0, 0, 0);
}
void Renderer::recordBackBufferEnd()
{
    VkCommandBuffer &commandBuffer = backBuffers[backBufferIndex].commandBuffer;
    vkCmdEndRenderPass(commandBuffer);

    VkResult res = vkEndCommandBuffer(commandBuffer);
    if (res != VK_SUCCESS)
        std::cerr << "Failed to record command buffer : " << res << std::endl;
}

void Renderer::submitBackBuffer()
{
    VkSemaphore waitSemaphores[] = {backBuffers[backBufferIndex].acquireSemaphore};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    VkSemaphore signalSemaphores[] = {backBuffers[backBufferIndex].renderSemaphore};
    VkSubmitInfo submitInfo = {.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
                               .waitSemaphoreCount = 1,
                               .pWaitSemaphores = waitSemaphores,
                               .pWaitDstStageMask = waitStages,
                               .commandBufferCount = 1,
                               .pCommandBuffers = &backBuffers[backBufferIndex].commandBuffer,
                               .signalSemaphoreCount = 1,
                               .pSignalSemaphores = signalSemaphores};

    VkResult res = vkQueueSubmit(device.graphicsQueue, 1, &submitInfo, backBuffers[backBufferIndex].inFlightFence);
    if (res != VK_SUCCESS)
        std::cerr << "Failed to submit draw command buffer : " << res << std::endl;
}

void Renderer::presentBackBuffer(uint32_t imageIndex)
{
    VkSwapchainKHR swapchains[] = {swapchain.handle};
    VkSemaphore signalSemaphores[] = {backBuffers[backBufferIndex].renderSemaphore};
    VkPresentInfoKHR presentInfo = {.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
                                    .waitSemaphoreCount = 1,
                                    .pWaitSemaphores = signalSemaphores,
                                    .swapchainCount = 1,
                                    .pSwapchains = swapchains,
                                    .pImageIndices = &imageIndex,
                                    .pResults = nullptr};

    VkResult res = vkQueuePresentKHR(device.presentQueue, &presentInfo);
    if (res != VK_SUCCESS)
        std::cerr << "Failed to present : " << res << std::endl;
}

void Renderer::swapBuffers()
{
    backBufferIndex = (backBufferIndex + 1) % bufferingType;
}
