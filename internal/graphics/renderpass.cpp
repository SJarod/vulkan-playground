#include <array>
#include <iostream>

#include "device.hpp"
#include "swapchain.hpp"

#include "renderpass.hpp"

RenderPass::RenderPass(const Device &device, const SwapChain &swapchain) : device(device), swapchain(swapchain)
{
    VkAttachmentDescription colorAttachment = {
        .format = swapchain.imageFormat,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        // load : what to do with the already existing image on the framebuffer
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        // store : what to do with the newly rendered image on the framebuffer
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
    };

    VkAttachmentReference colorAttachmentRef = {
        .attachment = 0, // colorAttachment is index 0
        .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
    };

    VkAttachmentDescription depthAttachment = {
        .format = swapchain.depthFormat,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
    };

    VkAttachmentReference depthAttachmentRef = {
        .attachment = 1,
        .layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
    };

    VkSubpassDescription subpass = {
        .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
        .colorAttachmentCount = 1,
        .pColorAttachments = &colorAttachmentRef,
        .pDepthStencilAttachment = &depthAttachmentRef,
    };

    VkSubpassDependency dependency = {
        .srcSubpass = VK_SUBPASS_EXTERNAL,
        .dstSubpass = 0,
        .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
        .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
        .srcAccessMask = 0,
        .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
    };

    std::array<VkAttachmentDescription, 2> attachments = {colorAttachment, depthAttachment};
    VkRenderPassCreateInfo createInfo = {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        .attachmentCount = static_cast<uint32_t>(attachments.size()),
        .pAttachments = attachments.data(),
        .subpassCount = 1,
        .pSubpasses = &subpass,
        .dependencyCount = 1,
        .pDependencies = &dependency,
    };

    VkResult res = vkCreateRenderPass(*device.handle, &createInfo, nullptr, &handle);
    if (res != VK_SUCCESS)
    {

        std::cerr << "Failed to create render pass : " << res << std::endl;
        return;
    }

    framebuffers.resize(swapchain.imageViews.size());

    for (size_t i = 0; i < swapchain.imageViews.size(); ++i)
    {
        std::array<VkImageView, 2> framebufferAttachments = {swapchain.imageViews[i], swapchain.depthImageView};
        VkFramebufferCreateInfo createInfo = {
            .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
            .renderPass = handle,
            .attachmentCount = static_cast<uint32_t>(framebufferAttachments.size()),
            .pAttachments = framebufferAttachments.data(),
            .width = swapchain.extent.width,
            .height = swapchain.extent.height,
            .layers = 1,
        };

        VkResult res = vkCreateFramebuffer(*device.handle, &createInfo, nullptr, &framebuffers[i]);
        if (res != VK_SUCCESS)
            std::cerr << "Failed to create framebuffer : " << res << std::endl;
    }
}

RenderPass::~RenderPass()
{
    for (VkFramebuffer &framebuffer : framebuffers)
    {
        vkDestroyFramebuffer(*device.handle, framebuffer, nullptr);
    }
    vkDestroyRenderPass(*device.handle, handle, nullptr);
}