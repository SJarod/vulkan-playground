#include "context.hpp"

Context::Context()
{
    m_loader = std::make_unique<Utils::bin::DynamicLibraryLoader>("vulkan-1");
    vkCreateInstance = (PFN_vkCreateInstance)m_loader->getProcAddr("vkCreateInstance");

    m_instance = std::make_unique<Instance>(*this);
}

int Context::enumerateAvailablePhysicalDevice() const
{
    return 0;
}