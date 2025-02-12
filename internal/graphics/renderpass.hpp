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
    std::weak_ptr<Device> device;

    VkRenderPass handle;
    std::vector<VkFramebuffer> framebuffers;

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
        return handle;
    }

    [[nodiscard]] const VkFramebuffer &getFramebuffer(uint32_t index) const
    {
        return framebuffers[index];
    }
};

class RenderPassBuilder
{
  private:
    std::unique_ptr<RenderPass> product;

    std::vector<VkAttachmentDescription> attachments;

    std::vector<VkAttachmentReference> colorAttachmentReferences;
    std::vector<VkAttachmentReference> depthAttachmentReferences;

    VkSubpassDescription subpass = {};
    VkSubpassDependency subpassDependency = {};

    std::weak_ptr<Device> device;
    std::weak_ptr<SwapChain> swapchain;

    void restart()
    {
        product = std::unique_ptr<RenderPass>(new RenderPass);

        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpassDependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        subpassDependency.dstSubpass = 0;
        subpassDependency.srcAccessMask = 0;
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
            .attachment = static_cast<uint32_t>(attachments.size()),
            .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        };

        attachments.emplace_back(colorAttachment);
        colorAttachmentReferences.emplace_back(colorAttachmentRef);

        subpassDependency.srcStageMask |= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        subpassDependency.dstStageMask |= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        subpassDependency.dstAccessMask |= VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
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
            .attachment = static_cast<uint32_t>(attachments.size()),
            .layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
        };

        attachments.emplace_back(depthAttachment);
        depthAttachmentReferences.emplace_back(depthAttachmentRef);

        subpassDependency.srcStageMask |= VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        subpassDependency.dstStageMask |= VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        subpassDependency.dstAccessMask |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    }

    void setDevice(std::weak_ptr<Device> device)
    {
        this->device = device;
        product->device = device;
    }
    void setSwapChain(std::weak_ptr<SwapChain> swapchain)
    {
        this->swapchain = swapchain;
    }

    std::unique_ptr<RenderPass> build();
};
