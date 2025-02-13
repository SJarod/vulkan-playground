#pragma once

#include <cassert>
#include <memory>
#include <optional>
#include <vector>

#include <vulkan/vulkan.h>

#include "surface.hpp"

class Context;

class Device
{
  private:
    std::weak_ptr<Context> m_cx;

    std::vector<const char *> m_deviceExtensions;

    const Surface *m_surface = nullptr;

    // physical device
    VkPhysicalDevice m_physicalHandle;
    VkPhysicalDeviceFeatures m_features;
    VkPhysicalDeviceProperties m_props;

    // logical device
    VkDevice m_handle;

    std::optional<uint32_t> m_graphicsFamilyIndex;
    std::optional<uint32_t> m_presentFamilyIndex;

    VkQueue m_graphicsQueue;
    VkQueue m_presentQueue;

    VkCommandPool m_commandPool;
    VkCommandPool m_commandPoolTransient;

  public:
    Device(const std::shared_ptr<Context> cx, VkPhysicalDevice base, const Surface *surface = nullptr);
    ~Device();

    Device(const Device &) = delete;
    Device &operator=(const Device &) = delete;
    Device(Device &&) = delete;
    Device &operator=(Device &&) = delete;

    void addDeviceExtension(const char *extension);

    std::vector<VkQueueFamilyProperties> getQueueFamilyProperties() const;

    std::optional<uint32_t> findQueueFamilyIndex(const VkQueueFlags &capabilities) const;
    std::optional<uint32_t> findPresentQueueFamilyIndex() const;

    void initLogicalDevice();

    std::optional<uint32_t> findMemoryTypeIndex(VkMemoryRequirements requirements,
                                                VkMemoryPropertyFlags properties) const;

    VkCommandBuffer cmdBeginOneTimeSubmit() const;
    void cmdEndOneTimeSubmit(VkCommandBuffer commandBuffer) const;

  public:
    [[nodiscard]] inline int getDeviceExtensionCount() const
    {
        return m_deviceExtensions.size();
    }
    [[nodiscard]] inline const char *const *getDeviceExtensions() const
    {
        return m_deviceExtensions.data();
    }
    [[nodiscard]] inline const VkPhysicalDeviceFeatures &getPhysicalDeviceFeatures()
    {
        return m_features;
    }
    [[nodiscard]] inline const VkPhysicalDeviceProperties &getPhysicalDeviceProperties()
    {
        return m_props;
    }

    [[nodiscard]] inline const VkPhysicalDevice &getPhysicalHandle() const
    {
        return m_physicalHandle;
    }
    [[nodiscard]] inline const VkDevice &getHandle() const
    {
        return m_handle;
    }

    [[nodiscard]] inline const VkCommandPool &getCommandPool() const
    {
        return m_commandPool;
    }

    [[nodiscard]] inline const VkSurfaceKHR getSurfaceHandle() const
    {
        assert(m_surface);
        return m_surface->getHandle();
    }

    [[nodiscard]] inline const std::optional<uint32_t> &getGraphicsFamilyIndex() const
    {
        return m_graphicsFamilyIndex;
    }
    [[nodiscard]] inline const std::optional<uint32_t> &getPresentFamilyIndex() const
    {
        return m_presentFamilyIndex;
    }

    [[nodiscard]] inline const VkQueue &getGraphicsQueue() const
    {
        return m_graphicsQueue;
    }

    [[nodiscard]] inline const VkQueue &getPresentQueue() const
    {
        return m_presentQueue;
    }
};