#include "graphics/device.hpp"
#include "graphics/renderpass.hpp"
#include "graphics/swapchain.hpp"

#include "renderer.hpp"

Renderer::Renderer(const Device &device, const SwapChain &swapchain) : device(device), swapchain(swapchain)
{
    renderPass = std::make_unique<RenderPass>(device, swapchain);
}

Renderer::~Renderer()
{
    renderPass.reset();
}
