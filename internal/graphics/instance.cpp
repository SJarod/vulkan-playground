#include "context.hpp"

#include "instance.hpp"

Instance::Instance(const Context& cx)
{
    VkApplicationInfo appInfo = {
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pApplicationName = "advanced_vulkan_test",
        .applicationVersion = VK_MAKE_API_VERSION(0, 0, 1, 0),
        .pEngineName = "engine",
        .engineVersion = VK_MAKE_API_VERSION(0, 0, 1, 0),
        .apiVersion = VK_API_VERSION_1_3,
    };
    VkInstanceCreateInfo createInfo = {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pApplicationInfo = &appInfo,
        .enabledLayerCount = static_cast<uint32_t>(cx.getLayerCount()),
        .ppEnabledLayerNames = cx.getLayers(),
        .enabledExtensionCount = static_cast<uint32_t>(cx.getInstanceExtensionCount()),
        .ppEnabledExtensionNames = cx.getInstanceExtensions(),
    };

    VkInstance handle;
    if (cx.vkCreateInstance(&createInfo, nullptr, &handle) != VK_SUCCESS)
        throw;

    m_handle = std::make_unique<VkInstance>(handle);
}