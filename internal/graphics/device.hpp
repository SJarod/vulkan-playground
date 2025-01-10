#pragma once

#include <optional>
#include <memory>
#include <vector>

#include <volk.h>

#include "surface.hpp"
#include "swapchain.hpp"

class Context;
class Surface;
class SwapChain;

typedef VkResult (*PFN_CreateSurfacePredicate)(VkInstance, const void *, VkAllocationCallbacks *, VkSurfaceKHR *);

class Device
{
  private:
    const Context &cx;

  public:
    // physical device
    VkPhysicalDevice physicalHandle;

    // logical device
    std::unique_ptr<VkDevice> handle;

    std::optional<uint32_t> graphicsFamilyIndex;
    std::optional<uint32_t> presentFamilyIndex;

  public:
    Device() = delete;
    Device(const Context &cx, VkPhysicalDevice base, const Surface *surface = nullptr);
    ~Device();

    std::vector<VkQueueFamilyProperties> getQueueFamilyProperties() const;

    std::optional<uint32_t> findQueueFamilyIndex(const VkQueueFlags &capabilities) const;
    std::optional<uint32_t> findPresentQueueFamilyIndex(const Surface *surface) const;

    void initLogicalDevice();

    std::unique_ptr<Surface> createSurface(PFN_CreateSurfacePredicate predicate, const void *windowHandle);
    std::unique_ptr<SwapChain> createSwapChain(const Surface &surface);
};