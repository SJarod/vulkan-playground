#include <iostream>
#include <set>
#include <vector>

#include "context.hpp"
#include "surface.hpp"
#include "swapchain.hpp"

#include "device.hpp"

Device::Device(const Context &cx, VkPhysicalDevice base, const Surface *surface) : cx(cx)
{
    physicalHandle = base;
    graphicsFamilyIndex = findQueueFamilyIndex(VK_QUEUE_GRAPHICS_BIT);
    if (surface)
        presentFamilyIndex = findPresentQueueFamilyIndex(surface);
}

Device::~Device()
{
    vkDestroyDevice(*handle, nullptr);
}

void Device::initLogicalDevice()
{
    std::set<uint32_t> uniqueQueueFamilies;

    if (graphicsFamilyIndex.has_value())
        uniqueQueueFamilies.insert(graphicsFamilyIndex.value());
    if (presentFamilyIndex.has_value())
        uniqueQueueFamilies.insert(presentFamilyIndex.value());

    float queuePriority = 1.f;
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    for (uint32_t queueFamily : uniqueQueueFamilies)
    {
        VkDeviceQueueCreateInfo queueCreateInfo = {.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                                                   .queueFamilyIndex = queueFamily,
                                                   .queueCount = 1,
                                                   .pQueuePriorities = &queuePriority};
        queueCreateInfos.emplace_back(queueCreateInfo);
    }

    VkDeviceCreateInfo createInfo = {
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size()),
        .pQueueCreateInfos = queueCreateInfos.data(),
        .enabledLayerCount = static_cast<uint32_t>(cx.getLayerCount()),
        .ppEnabledLayerNames = cx.getLayers(),
        .enabledExtensionCount = static_cast<uint32_t>(cx.getDeviceExtensionCount()),
        .ppEnabledExtensionNames = cx.getDeviceExtensions(),
    };

    // create device
    VkDevice handle;
    if (vkCreateDevice(physicalHandle, &createInfo, nullptr, &handle) != VK_SUCCESS)
        std::cerr << "Failed to create logical device" << std::endl;
    else
        this->handle = std::make_unique<VkDevice>(handle);
}


std::unique_ptr<SwapChain> Device::createSwapChain(const Surface &surface)
{
    VkSurfaceCapabilitiesKHR capabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalHandle, surface.getHandle(), &capabilities);

    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(physicalHandle, surface.getHandle(), &formatCount, nullptr);
    std::vector<VkSurfaceFormatKHR> formats(formatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(physicalHandle, surface.getHandle(), &formatCount, formats.data());

    uint32_t modeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(physicalHandle, surface.getHandle(), &modeCount, nullptr);
    std::vector<VkPresentModeKHR> presentModes(modeCount);
    vkGetPhysicalDeviceSurfacePresentModesKHR(physicalHandle, surface.getHandle(), &modeCount, presentModes.data());

    VkSurfaceFormatKHR surfaceFormat = formats[0];
    VkPresentModeKHR presentMode = presentModes[0];
    VkExtent2D extent = {1366, 768};

    uint32_t imageCount = capabilities.minImageCount + 1;
    if (capabilities.maxImageCount > 0 && capabilities.maxImageCount < imageCount)
        imageCount = capabilities.maxImageCount;

    VkSwapchainCreateInfoKHR createInfo = {.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
                                           .surface = surface.getHandle(),
                                           .minImageCount = imageCount,
                                           .imageFormat = surfaceFormat.format,
                                           .imageColorSpace = surfaceFormat.colorSpace,
                                           .imageExtent = extent,
                                           .imageArrayLayers = 1,
                                           .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                                           .preTransform = capabilities.currentTransform,
                                           .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
                                           .presentMode = presentMode,
                                           .clipped = VK_TRUE,
                                           .oldSwapchain = VK_NULL_HANDLE};

    uint32_t queueFamilyIndices[] = {graphicsFamilyIndex.value(), presentFamilyIndex.value()};

    if (graphicsFamilyIndex != presentFamilyIndex)
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    }
    else
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0;
        createInfo.pQueueFamilyIndices = nullptr;
    }

    VkSwapchainKHR swapchain;
    if (vkCreateSwapchainKHR(*handle, &createInfo, nullptr, &swapchain) != VK_SUCCESS)
        throw std::exception("Failed to create swapchain");

    std::unique_ptr<SwapChain> out = std::make_unique<SwapChain>(*this, swapchain);
    out->imageFormat = surfaceFormat.format;
    out->extent = extent;

    return std::move(out);
}

std::vector<VkQueueFamilyProperties> Device::getQueueFamilyProperties() const
{
    uint32_t queueFamilyPropertiesCount;
    vkGetPhysicalDeviceQueueFamilyProperties(physicalHandle, &queueFamilyPropertiesCount, nullptr);
    std::vector<VkQueueFamilyProperties> out(queueFamilyPropertiesCount);
    vkGetPhysicalDeviceQueueFamilyProperties(physicalHandle, &queueFamilyPropertiesCount, out.data());
    return out;
}

std::optional<uint32_t> Device::findQueueFamilyIndex(const VkQueueFlags &capabilities) const
{
    auto props = getQueueFamilyProperties();
    for (uint32_t i = 0; i < props.size(); ++i)
    {
        if (props[i].queueFlags & capabilities)
            return std::optional<uint32_t>(i);
    }
    return std::optional<uint32_t>();
}
std::optional<uint32_t> Device::findPresentQueueFamilyIndex(const Surface *surface) const
{
    if (!surface)
        return std::optional<uint32_t>();

    auto props = getQueueFamilyProperties();
    for (uint32_t i = 0; i < props.size(); ++i)
    {
        VkBool32 supported;
        vkGetPhysicalDeviceSurfaceSupportKHR(physicalHandle, i, surface->getHandle(), &supported);
        if (supported)
            return std::optional<uint32_t>(i);
    }
    return std::optional<uint32_t>();
}