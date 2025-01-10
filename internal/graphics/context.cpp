#include <iostream>
#include <set>
#include "device.hpp"

#include "context.hpp"

Context::Context()
{
    volkInitialize();

    m_instance = std::make_unique<Instance>(*this);

    volkLoadInstance(m_instance->getInstance());
}

std::vector<VkPhysicalDevice> Context::getAvvailablePhysicalDevices() const
{
    uint32_t count;
    vkEnumeratePhysicalDevices(m_instance->getInstance(), &count, nullptr);
    std::vector<VkPhysicalDevice> physicalDevices(count);
    vkEnumeratePhysicalDevices(m_instance->getInstance(), &count, physicalDevices.data());
    return physicalDevices;
}