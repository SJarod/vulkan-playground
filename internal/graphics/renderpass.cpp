#include <array>
#include <cassert>
#include <iostream>

#include "device.hpp"
#include "swapchain.hpp"

#include "renderpass.hpp"

RenderPass::~RenderPass()
{
    if (!device.lock())
        return;

    const VkDevice &deviceHandle = device.lock()->getHandle();

    for (VkFramebuffer &framebuffer : framebuffers)
    {
        vkDestroyFramebuffer(deviceHandle, framebuffer, nullptr);
    }
    vkDestroyRenderPass(deviceHandle, handle, nullptr);
}

std::unique_ptr<RenderPass> RenderPassBuilder::build()
{
    assert(device.lock());
    assert(swapchain.lock());

    const VkDevice &deviceHandle = device.lock()->getHandle();
    auto swapchainPtr = swapchain.lock();

    subpass.colorAttachmentCount = static_cast<uint32_t>(colorAttachmentReferences.size());
    subpass.pColorAttachments = colorAttachmentReferences.data();
    subpass.pDepthStencilAttachment = depthAttachmentReferences.data();

    VkRenderPassCreateInfo createInfo = {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        .attachmentCount = static_cast<uint32_t>(attachments.size()),
        .pAttachments = attachments.data(),
        .subpassCount = 1,
        .pSubpasses = &subpass,
        .dependencyCount = 1,
        .pDependencies = &subpassDependency,
    };

    VkRenderPass handle;
    VkResult res = vkCreateRenderPass(deviceHandle, &createInfo, nullptr, &handle);
    if (res != VK_SUCCESS)
    {
        std::cerr << "Failed to create render pass : " << res << std::endl;
        return nullptr;
    }

    product->handle = handle;

    product->framebuffers.resize(swapchainPtr->imageViews.size());

    for (size_t i = 0; i < swapchainPtr->imageViews.size(); ++i)
    {
        std::array<VkImageView, 2> framebufferAttachments = {swapchainPtr->imageViews[i], swapchainPtr->depthImageView};
        VkFramebufferCreateInfo createInfo = {
            .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
            .renderPass = handle,
            .attachmentCount = static_cast<uint32_t>(framebufferAttachments.size()),
            .pAttachments = framebufferAttachments.data(),
            .width = swapchainPtr->extent.width,
            .height = swapchainPtr->extent.height,
            .layers = 1,
        };

        VkResult res = vkCreateFramebuffer(deviceHandle, &createInfo, nullptr, &product->framebuffers[i]);
        if (res != VK_SUCCESS)
            std::cerr << "Failed to create framebuffer : " << res << std::endl;
    }

    auto result = std::move(product);
    return result;
}
