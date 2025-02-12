#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

#include "graphics/buffer.hpp"
#include "graphics/device.hpp"
#include "graphics/pipeline.hpp"
#include "graphics/swapchain.hpp"

#include "mesh.hpp"
#include "texture.hpp"

#include "engine/camera.hpp"
#include "engine/uniform.hpp"

#include "renderstate.hpp"

#include "renderer.hpp"

Renderer::~Renderer()
{
    if (!device.lock())
        return;

    auto deviceHandle = device.lock()->getHandle();

    vkDeviceWaitIdle(deviceHandle);

    for (int i = 0; i < bufferingType; ++i)
    {
        vkDestroyFence(deviceHandle, backBuffers[i].inFlightFence, nullptr);
        vkDestroySemaphore(deviceHandle, backBuffers[i].renderSemaphore, nullptr);
        vkDestroySemaphore(deviceHandle, backBuffers[i].acquireSemaphore, nullptr);
    }

    renderPass.reset();
}

void Renderer::registerRenderState(const std::shared_ptr<Mesh> mesh)
{
    MeshRenderStateBuilder mrsb;
    mrsb.setDevice(device);
    mrsb.setTexture(mesh->texture.get());
    RenderStateDirector rsd;
    mrsb.setFrameInFlightCount(swapchain.lock()->frameInFlightCount);
    rsd.createUniformAndSamplerRenderStateBuilder(mrsb);
    mrsb.setMesh(mesh);
    PipelineBuilder pb;
    PipelineDirector pd;
    pd.createColorDepthRasterizerBuilder(pb);
    pb.setDevice(device);
    pb.addVertexShaderStage("triangle");
    pb.addFragmentShaderStage("triangle");
    pb.setRenderPass(renderPass.get());
    pb.setExtent(swapchain.lock()->extent);
    mrsb.setPipeline(pb.build());
    renderStates.emplace_back(mrsb.build());
}

uint32_t Renderer::acquireBackBuffer()
{
    auto deviceHandle = device.lock()->getHandle();

    vkWaitForFences(deviceHandle, 1, &backBuffers[backBufferIndex].inFlightFence, VK_TRUE, UINT64_MAX);
    vkResetFences(deviceHandle, 1, &backBuffers[backBufferIndex].inFlightFence);

    uint32_t imageIndex;
    VkResult res = vkAcquireNextImageKHR(deviceHandle, swapchain.lock()->getHandle(), UINT64_MAX,
                                         backBuffers[backBufferIndex].acquireSemaphore, VK_NULL_HANDLE, &imageIndex);
    if (res != VK_SUCCESS)
    {
        std::cerr << "Failed to acquire next image : " << res << std::endl;
        return -1;
    }

    return imageIndex;
}

void Renderer::recordRenderers(uint32_t imageIndex, const Camera &camera)
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

    VkClearValue clearColor = {
        .color = {0.2f, 0.2f, 0.2f, 1.f},
    };
    VkClearValue clearDepth = {
        .depthStencil = {1.f, 0},
    };
    std::array<VkClearValue, 2> clearValues = {clearColor, clearDepth};
    VkRenderPassBeginInfo renderPassBeginInfo = {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .renderPass = renderPass->getHandle(),
        .framebuffer = renderPass->getFramebuffer(imageIndex),
        .renderArea =
            {
                .offset = {0, 0},
                .extent = swapchain.lock()->extent,
            },
        .clearValueCount = static_cast<uint32_t>(clearValues.size()),
        .pClearValues = clearValues.data(),
    };
    vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

    for (int i = 0; i < renderStates.size(); ++i)
    {
        renderStates[i]->updateUniformBuffers(imageIndex, camera);

        renderStates[i]->getPipeline()->recordBind(commandBuffer, imageIndex);

        renderStates[i]->recordBackBufferDescriptorSetsCommands(commandBuffer, imageIndex);
        renderStates[i]->recordBackBufferDrawObjectCommands(commandBuffer);
    }

    vkCmdEndRenderPass(commandBuffer);

    res = vkEndCommandBuffer(commandBuffer);
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

    VkResult res =
        vkQueueSubmit(device.lock()->graphicsQueue, 1, &submitInfo, backBuffers[backBufferIndex].inFlightFence);
    if (res != VK_SUCCESS)
        std::cerr << "Failed to submit draw command buffer : " << res << std::endl;
}

void Renderer::presentBackBuffer(uint32_t imageIndex)
{
    VkSwapchainKHR swapchains[] = {swapchain.lock()->getHandle()};
    VkSemaphore waitSemaphores[] = {backBuffers[backBufferIndex].renderSemaphore};
    VkPresentInfoKHR presentInfo = {.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
                                    .waitSemaphoreCount = 1,
                                    .pWaitSemaphores = waitSemaphores,
                                    .swapchainCount = 1,
                                    .pSwapchains = swapchains,
                                    .pImageIndices = &imageIndex,
                                    .pResults = nullptr};

    VkResult res = vkQueuePresentKHR(device.lock()->presentQueue, &presentInfo);
    if (res != VK_SUCCESS)
        std::cerr << "Failed to present : " << res << std::endl;
}

void Renderer::swapBuffers()
{
    backBufferIndex = (backBufferIndex + 1) % bufferingType;
}

std::unique_ptr<Renderer> RendererBuilder::build()
{
    assert(device.lock());
    assert(swapchain.lock());

    auto devicePtr = device.lock();
    auto deviceHandle = devicePtr->getHandle();

    RenderPassBuilder rpb;
    rpb.setDevice(device);
    rpb.setSwapChain(swapchain);
    rpb.addColorAttachment(swapchain.lock()->imageFormat);
    rpb.addDepthAttachment(swapchain.lock()->depthFormat);
    product->renderPass = rpb.build();

    // back buffers

    product->backBuffers.resize(product->bufferingType);

    VkCommandBufferAllocateInfo commandBufferAllocInfo = {.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
                                                          .commandPool = devicePtr->getCommandPool(),
                                                          .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
                                                          .commandBufferCount = 1U};
    for (int i = 0; i < product->bufferingType; ++i)
    {
        VkResult res =
            vkAllocateCommandBuffers(deviceHandle, &commandBufferAllocInfo, &product->backBuffers[i].commandBuffer);
        if (res != VK_SUCCESS)
        {
            std::cerr << "Failed to allocate command buffers : " << res << std::endl;
            return nullptr;
        }
    }

    // synchronization

    VkSemaphoreCreateInfo semaphoreCreateInfo = {.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
    VkFenceCreateInfo fenceCreateInfo = {.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
                                         .flags = VK_FENCE_CREATE_SIGNALED_BIT};
    for (int i = 0; i < product->bufferingType; ++i)
    {
        VkResult res =
            vkCreateSemaphore(deviceHandle, &semaphoreCreateInfo, nullptr, &product->backBuffers[i].acquireSemaphore);
        if (res != VK_SUCCESS)
        {
            std::cerr << "Failed to create semaphore : " << res << std::endl;
            return nullptr;
        }

        res = vkCreateSemaphore(deviceHandle, &semaphoreCreateInfo, nullptr, &product->backBuffers[i].renderSemaphore);
        if (res != VK_SUCCESS)
        {
            std::cerr << "Failed to create semaphore : " << res << std::endl;
            return nullptr;
        }

        res = vkCreateFence(deviceHandle, &fenceCreateInfo, nullptr, &product->backBuffers[i].inFlightFence);
        if (res != VK_SUCCESS)
        {
            std::cerr << "Failed to create fence : " << res << std::endl;
            return nullptr;
        }
    }

    auto result = std::move(product);
    return result;
}