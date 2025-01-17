#pragma once

#include <optional>
#include <memory>
#include <vector>

#include <vulkan/vulkan.h>

#include "surface.hpp"
#include "swapchain.hpp"

class Context;
class Surface;
class SwapChain;

class Device
{
  private:
    const Context &cx;

public:
    const Surface* surface = nullptr;

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
    std::optional<uint32_t> findPresentQueueFamilyIndex() const;

    void initLogicalDevice();
};