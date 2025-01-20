#pragma once

#include <memory>
#include <optional>
#include <vector>

#include <vulkan/vulkan.h>

#include "surface.hpp"

class Context;

class Device
{
  private:
    const Context &cx;

  public:
    const Surface *surface = nullptr;

  public:
    // physical device
    VkPhysicalDevice physicalHandle;

    // logical device
    std::unique_ptr<VkDevice> handle;

    std::optional<uint32_t> graphicsFamilyIndex;
    std::optional<uint32_t> presentFamilyIndex;

    VkQueue graphicsQueue;
    VkQueue presentQueue;

  public:
    Device() = delete;
    Device(const Context &cx, VkPhysicalDevice base, const Surface *surface = nullptr);
    ~Device();

    std::vector<VkQueueFamilyProperties> getQueueFamilyProperties() const;

    std::optional<uint32_t> findQueueFamilyIndex(const VkQueueFlags &capabilities) const;
    std::optional<uint32_t> findPresentQueueFamilyIndex() const;

    void initLogicalDevice();

    std::optional<uint32_t> findMemoryTypeIndex(VkMemoryRequirements requirements, VkMemoryPropertyFlags properties) const;
};