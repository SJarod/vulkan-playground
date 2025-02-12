#pragma once

#include <memory>

#include "graphics/renderpass.hpp"

class Device;
class SwapChain;
class Pipeline;
class Mesh;
class Texture;
class Buffer;
class Camera;
class RenderState;

struct BackBufferT
{
    VkCommandBuffer commandBuffer;

    VkSemaphore acquireSemaphore;
    VkSemaphore renderSemaphore;
    VkFence inFlightFence;
};

class RendererBuilder;

class Renderer
{
    friend RendererBuilder;

  private:
    std::weak_ptr<Device> device;
    std::weak_ptr<SwapChain> swapchain;

  public:
    int bufferingType = 2;

    std::unique_ptr<RenderPass> renderPass;

    std::vector<std::shared_ptr<RenderState>> renderStates;

    int backBufferIndex = 0;
    std::vector<BackBufferT> backBuffers;

    Renderer() = default;

  public:
    ~Renderer();

    void registerRenderState(const std::shared_ptr<Mesh> mesh);

    uint32_t acquireBackBuffer();

    void recordRenderers(uint32_t imageIndex, const Camera &camera);

    void submitBackBuffer();
    void presentBackBuffer(uint32_t imageIndex);

    void swapBuffers();
};

class RendererBuilder
{
  private:
    std::unique_ptr<Renderer> product;

    std::weak_ptr<Device> device;
    std::weak_ptr<SwapChain> swapchain;

    void restart()
    {
        product = std::unique_ptr<Renderer>(new Renderer);
        product->bufferingType = 2U;
    }

  public:
    RendererBuilder()
    {
        restart();
    }

    void setDevice(std::weak_ptr<Device> device)
    {
        this->device = device;
        product->device = device;
    }
    void setSwapChain(std::weak_ptr<SwapChain> swapchain)
    {
        this->swapchain = swapchain;
        product->swapchain = swapchain;
    }
    void setBufferingType(uint32_t type)
    {
        product->bufferingType = type;
    }

    std::unique_ptr<Renderer> build();
};