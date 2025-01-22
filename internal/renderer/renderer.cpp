#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

#include "graphics/buffer.hpp"
#include "graphics/device.hpp"
#include "graphics/pipeline.hpp"
#include "graphics/renderpass.hpp"
#include "graphics/swapchain.hpp"

#include "mesh.hpp"
#include "texture.hpp"

#include "engine/uniform.hpp"

#include "renderer.hpp"

Renderer::Renderer(const Device &device, const SwapChain &swapchain, const int bufferingType)
    : device(device), swapchain(swapchain), bufferingType(bufferingType)
{
    renderPass = std::make_unique<RenderPass>(device, swapchain);

    pipeline = std::make_unique<Pipeline>(device, "triangle", *renderPass, swapchain.extent);

    // descriptor pool

    std::vector<VkDescriptorPoolSize> poolSizes = {VkDescriptorPoolSize{
                                                       .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                                                       .descriptorCount = renderPass->swapchain.frameInFlightCount,
                                                   },
                                                   VkDescriptorPoolSize{
                                                       .type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                                       .descriptorCount = renderPass->swapchain.frameInFlightCount,
                                                   }};
    VkDescriptorPoolCreateInfo createInfo = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .maxSets = renderPass->swapchain.frameInFlightCount,
        .poolSizeCount = static_cast<uint32_t>(poolSizes.size()),
        .pPoolSizes = poolSizes.data(),
    };
    VkResult res = vkCreateDescriptorPool(*device.handle, &createInfo, nullptr, &descriptorPool);
    if (res != VK_SUCCESS)
    {
        std::cerr << "Failed to create descriptor pool : " << res << std::endl;
        return;
    }

    // descriptor sets

    std::vector<VkDescriptorSetLayout> setLayouts(renderPass->swapchain.frameInFlightCount,
                                                  pipeline->descriptorSetLayout);
    VkDescriptorSetAllocateInfo descriptorSetAllocInfo = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .descriptorPool = descriptorPool,
        .descriptorSetCount = renderPass->swapchain.frameInFlightCount,
        .pSetLayouts = setLayouts.data(),
    };
    descriptorSets.resize(renderPass->swapchain.frameInFlightCount);
    res = vkAllocateDescriptorSets(*device.handle, &descriptorSetAllocInfo, descriptorSets.data());
    if (res != VK_SUCCESS)
    {
        std::cerr << "Failed to allocate descriptor sets : " << res << std::endl;
        return;
    }

    // uniform buffers

    uniformBuffers.resize(renderPass->swapchain.frameInFlightCount);
    uniformBuffersMapped.resize(renderPass->swapchain.frameInFlightCount);
    for (int i = 0; i < uniformBuffers.size(); ++i)
    {
        uniformBuffers[i] =
            std::make_unique<Buffer>(device, sizeof(UniformBufferObject), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        vkMapMemory(*device.handle, uniformBuffers[i]->memory, 0, sizeof(UniformBufferObject), 0,
                    &uniformBuffersMapped[i]);
    }

    // back buffers

    backBuffers.resize(bufferingType);

    VkCommandBufferAllocateInfo commandBufferAllocInfo = {.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
                                                          .commandPool = device.commandPool,
                                                          .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
                                                          .commandBufferCount = 1U};
    for (int i = 0; i < bufferingType; ++i)
    {
        res = vkAllocateCommandBuffers(*device.handle, &commandBufferAllocInfo, &backBuffers[i].commandBuffer);
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
        res = vkCreateSemaphore(*device.handle, &semaphoreCreateInfo, nullptr, &backBuffers[i].acquireSemaphore);
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
    vkDeviceWaitIdle(*device.handle);

    for (int i = 0; i < bufferingType; ++i)
    {
        vkDestroyFence(*device.handle, backBuffers[i].inFlightFence, nullptr);
        vkDestroySemaphore(*device.handle, backBuffers[i].renderSemaphore, nullptr);
        vkDestroySemaphore(*device.handle, backBuffers[i].acquireSemaphore, nullptr);
    }

    vkDestroyDescriptorPool(*device.handle, descriptorPool, nullptr);

    pipeline.reset();
    renderPass.reset();
}

void Renderer::writeDescriptorSets(const Texture &texture)
{
    for (int i = 0; i < descriptorSets.size(); ++i)
    {
        VkDescriptorBufferInfo bufferInfo = {
            .buffer = uniformBuffers[i]->handle,
            .offset = 0,
            .range = sizeof(UniformBufferObject),
        };
        VkDescriptorImageInfo imageInfo = {
            .sampler = texture.sampler,
            .imageView = texture.imageView,
            .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        };
        std::vector<VkWriteDescriptorSet> writes =
            UniformBufferObject::get_uniform_descriptor_set_writes(descriptorSets[i], bufferInfo, imageInfo);
        vkUpdateDescriptorSets(*device.handle, static_cast<uint32_t>(writes.size()), writes.data(), 0, nullptr);
    }
}

void Renderer::updateUniformBuffers(uint32_t imageIndex)
{
    UniformBufferObject ubo = {
        .model = glm::mat4(1.f),
        .view = glm::lookAt(glm::vec3(0.f, 1.f, 1.f), glm::vec3(0.f), glm::vec3(0.f, 1.f, 0.)),
        .proj = glm::perspective(glm::radians(45.f),
                                 renderPass->swapchain.extent.width / (float)renderPass->swapchain.extent.height, 0.1f,
                                 1000.f),
    };
    memcpy(uniformBuffersMapped[imageIndex], &ubo, sizeof(ubo));
}

uint32_t Renderer::acquireBackBuffer()
{
    vkWaitForFences(*device.handle, 1, &backBuffers[backBufferIndex].inFlightFence, VK_TRUE, UINT64_MAX);
    vkResetFences(*device.handle, 1, &backBuffers[backBufferIndex].inFlightFence);

    uint32_t imageIndex;
    VkResult res = vkAcquireNextImageKHR(*device.handle, swapchain.handle, UINT64_MAX,
                                         backBuffers[backBufferIndex].acquireSemaphore, VK_NULL_HANDLE, &imageIndex);
    if (res != VK_SUCCESS)
    {
        std::cerr << "Failed to acquire next image : " << res << std::endl;
        return -1;
    }

    return imageIndex;
}

void Renderer::recordBackBufferBeginRenderPass(uint32_t imageIndex)
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
void Renderer::recordBackBufferDescriptorSetsCommands(uint32_t imageIndex)
{
    vkCmdBindDescriptorSets(backBuffers[backBufferIndex].commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                            pipeline->pipelineLayout, 0, 1, &descriptorSets[imageIndex], 0, nullptr);
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
void Renderer::recordBackBufferEndRenderPass()
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
    VkSemaphore waitSemaphores[] = {backBuffers[backBufferIndex].renderSemaphore};
    VkPresentInfoKHR presentInfo = {.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
                                    .waitSemaphoreCount = 1,
                                    .pWaitSemaphores = waitSemaphores,
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
