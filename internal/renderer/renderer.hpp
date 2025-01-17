#pragma once

#include <memory>

class Device;
class RenderPass;
class SwapChain;

class Renderer
{
  private:
    const Device &device;
    const SwapChain &swapchain;

  public:
    std::unique_ptr<RenderPass> renderPass;

  public:
    Renderer(const Device &device, const SwapChain &swapchain);
    ~Renderer();
};