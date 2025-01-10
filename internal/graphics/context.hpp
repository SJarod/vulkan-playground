#pragma once

#include <vector>
#include <memory>

#include <volk.h>

#include "instance.hpp"

#define PFN_DECLARE_VK(funcName) PFN_##funcName funcName

class Context
{
private:
    std::vector<const char*> m_layers;
    std::vector<const char*> m_instanceExtensions;

public:
    std::unique_ptr<Instance> m_instance;

public:
    Context();

    inline int getLayerCount() const
    { return m_layers.size(); }
    inline const char* const* getLayers() const
    { return m_layers.data(); }
    inline int getInstanceExtensionCount() const
    { return m_instanceExtensions.size(); }
    inline const char* const* getInstanceExtensions() const
    { return m_instanceExtensions.data(); }

    /**
     * return the number of physical devices
    */
    int enumerateAvailablePhysicalDevice() const;

};