#include <array>
#include <cassert>
#include <iostream>

#include "device.hpp"
#include "swapchain.hpp"

#include "render_pass.hpp"

RenderPass::~RenderPass()
{
    if (!m_device.lock())
        return;

    const VkDevice &deviceHandle = m_device.lock()->getHandle();

    for (VkFramebuffer &framebuffer : m_framebuffers)
    {
        vkDestroyFramebuffer(deviceHandle, framebuffer, nullptr);
    }
    vkDestroyRenderPass(deviceHandle, m_handle, nullptr);
}

std::unique_ptr<RenderPass> RenderPassBuilder::build()
{
    assert(m_device.lock());
    assert(m_swapchain);

    const VkDevice &deviceHandle = m_device.lock()->getHandle();

    m_subpass.colorAttachmentCount = static_cast<uint32_t>(m_colorAttachmentReferences.size());
    m_subpass.pColorAttachments = m_colorAttachmentReferences.data();
    m_subpass.pDepthStencilAttachment = m_depthAttachmentReferences.data();

    VkRenderPassCreateInfo createInfo = {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        .attachmentCount = static_cast<uint32_t>(m_attachments.size()),
        .pAttachments = m_attachments.data(),
        .subpassCount = 1,
        .pSubpasses = &m_subpass,
        .dependencyCount = 1,
        .pDependencies = &m_subpassDependency,
    };

    VkRenderPass handle;
    VkResult res = vkCreateRenderPass(deviceHandle, &createInfo, nullptr, &handle);
    if (res != VK_SUCCESS)
    {
        std::cerr << "Failed to create render pass : " << res << std::endl;
        return nullptr;
    }

    m_product->m_handle = handle;

    auto imageViews = m_swapchain->getImageViews();
    m_product->m_framebuffers.resize(imageViews.size());

    for (size_t i = 0; i < imageViews.size(); ++i)
    {
        std::array<VkImageView, 2> framebufferAttachments = {imageViews[i], m_swapchain->getDepthImageView()};
        VkFramebufferCreateInfo createInfo = {
            .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
            .renderPass = handle,
            .attachmentCount = static_cast<uint32_t>(framebufferAttachments.size()),
            .pAttachments = framebufferAttachments.data(),
            .width = m_swapchain->getExtent().width,
            .height = m_swapchain->getExtent().height,
            .layers = 1,
        };

        VkResult res = vkCreateFramebuffer(deviceHandle, &createInfo, nullptr, &m_product->m_framebuffers[i]);
        if (res != VK_SUCCESS)
            std::cerr << "Failed to create framebuffer : " << res << std::endl;
    }

    auto result = std::move(m_product);
    return result;
}
