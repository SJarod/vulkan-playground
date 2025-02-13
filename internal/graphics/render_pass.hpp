#pragma once

#include <vector>

#include <vulkan/vulkan.h>

class Device;
class SwapChain;
class RenderPassBuilder;

class RenderPass
{
    friend RenderPassBuilder;

  private:
    std::weak_ptr<Device> m_device;

    VkRenderPass m_handle;
    std::vector<VkFramebuffer> m_framebuffers;

    RenderPass() = default;

  public:
    ~RenderPass();

    RenderPass(const RenderPass &) = delete;
    RenderPass &operator=(const RenderPass &) = delete;
    RenderPass(RenderPass &&) = delete;
    RenderPass &operator=(RenderPass &&) = delete;

  public:
    [[nodiscard]] const VkRenderPass &getHandle() const
    {
        return m_handle;
    }

    [[nodiscard]] const VkFramebuffer &getFramebuffer(uint32_t index) const
    {
        return m_framebuffers[index];
    }
};

class RenderPassBuilder
{
  private:
    std::unique_ptr<RenderPass> m_product;

    std::vector<VkAttachmentDescription> m_attachments;

    std::vector<VkAttachmentReference> m_colorAttachmentReferences;
    std::vector<VkAttachmentReference> m_depthAttachmentReferences;

    VkSubpassDescription m_subpass = {};
    VkSubpassDependency m_subpassDependency = {};

    std::weak_ptr<Device> m_device;
    const SwapChain* m_swapchain;

    void restart()
    {
        m_product = std::unique_ptr<RenderPass>(new RenderPass);

        m_subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        m_subpassDependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        m_subpassDependency.dstSubpass = 0;
        m_subpassDependency.srcAccessMask = 0;
    }

  public:
    RenderPassBuilder()
    {
        restart();
    }

    void addColorAttachment(VkFormat imageFormat)
    {
        VkAttachmentDescription colorAttachment = {
            .format = imageFormat,
            .samples = VK_SAMPLE_COUNT_1_BIT,
            .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
            .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
            .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
            .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
        };

        VkAttachmentReference colorAttachmentRef = {
            .attachment = static_cast<uint32_t>(m_attachments.size()),
            .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        };

        m_attachments.emplace_back(colorAttachment);
        m_colorAttachmentReferences.emplace_back(colorAttachmentRef);

        m_subpassDependency.srcStageMask |= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        m_subpassDependency.dstStageMask |= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        m_subpassDependency.dstAccessMask |= VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    }
    void addDepthAttachment(VkFormat depthImageFormat)
    {
        VkAttachmentDescription depthAttachment = {
            .format = depthImageFormat,
            .samples = VK_SAMPLE_COUNT_1_BIT,
            .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
            .storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
            .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
            .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            .finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
        };

        VkAttachmentReference depthAttachmentRef = {
            .attachment = static_cast<uint32_t>(m_attachments.size()),
            .layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
        };

        m_attachments.emplace_back(depthAttachment);
        m_depthAttachmentReferences.emplace_back(depthAttachmentRef);

        m_subpassDependency.srcStageMask |= VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        m_subpassDependency.dstStageMask |= VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        m_subpassDependency.dstAccessMask |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    }

    void setDevice(std::weak_ptr<Device> device)
    {
        m_device = device;
        m_product->m_device = device;
    }
    void setSwapChain(const SwapChain *swapchain)
    {
        m_swapchain = swapchain;
    }

    std::unique_ptr<RenderPass> build();
};
