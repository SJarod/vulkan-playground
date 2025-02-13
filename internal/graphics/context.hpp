#pragma once

#include <memory>
#include <vector>

#include <vulkan/vulkan.h>

#include "instance.hpp"

#define PFN_DECLARE_VK(funcName) PFN_##funcName funcName

class Device;

class Context
{
  private:
    std::vector<const char *> m_layers;
    std::vector<const char *> m_instanceExtensions;

    std::unique_ptr<Instance> m_instance;

  public:
    void finishCreateContext();

    void addLayer(const char *layer);
    void addInstanceExtension(const char *extension);

  public:
    [[nodiscard]] inline int getLayerCount() const
    {
        return m_layers.size();
    }
    [[nodiscard]] inline const char *const *getLayers() const
    {
        return m_layers.data();
    }
    [[nodiscard]] inline int getInstanceExtensionCount() const
    {
        return m_instanceExtensions.size();
    }
    [[nodiscard]] inline const char *const *getInstanceExtensions() const
    {
        return m_instanceExtensions.data();
    }

    [[nodiscard]] inline VkInstance getInstanceHandle() const
    {
        return m_instance->getHandle();
    }

    [[nodiscard]] std::vector<VkPhysicalDevice> getAvailablePhysicalDevices() const;
};