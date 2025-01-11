#include "context.hpp"
#include <iostream>

#include "instance.hpp"

Instance::Instance(const Context &cx)
{
    VkApplicationInfo appInfo = {
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pApplicationName = "rhi_sandbox",
        .applicationVersion = VK_MAKE_API_VERSION(0, 0, 1, 0),
        .pEngineName = "engine",
        .engineVersion = VK_MAKE_API_VERSION(0, 0, 1, 0),
        .apiVersion = VK_API_VERSION_1_3,
    };

    VkDebugReportCallbackCreateInfoEXT debugReportCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT,
        .flags = VK_DEBUG_REPORT_INFORMATION_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT |
                 VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT | VK_DEBUG_REPORT_ERROR_BIT_EXT |
                 VK_DEBUG_REPORT_DEBUG_BIT_EXT,
        .pfnCallback = &Instance::debugReportCallback,
    };
    VkInstanceCreateInfo createInfo = {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pNext = &debugReportCreateInfo,
        .pApplicationInfo = &appInfo,
        .enabledLayerCount = static_cast<uint32_t>(cx.getLayerCount()),
        .ppEnabledLayerNames = cx.getLayers(),
        .enabledExtensionCount = static_cast<uint32_t>(cx.getInstanceExtensionCount()),
        .ppEnabledExtensionNames = cx.getInstanceExtensions(),
    };

    uint32_t count;
    vkEnumerateInstanceLayerProperties(&count, nullptr);
    std::vector<VkLayerProperties> props(count);
    vkEnumerateInstanceLayerProperties(&count, props.data());
    for (int i = 0; i < count; ++i)
    {
        std::cout << props[i].layerName << std::endl;
    }

    VkInstance handle;
    if (vkCreateInstance(&createInfo, nullptr, &handle) != VK_SUCCESS)
    {
        std::cerr << "Failed to create instance" << std::endl;
        return;
    }

    m_handle = std::make_unique<VkInstance>(handle);
}

Instance::~Instance()
{
    if (m_handle)
        vkDestroyInstance(*m_handle, nullptr);
}