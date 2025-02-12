#include <iostream>

#include "device.hpp"
#include "image.hpp"

#include "swapchain.hpp"

SwapChain::SwapChain(const Device &device) : device(device)
{
    VkPhysicalDevice physicalHandle = device.physicalHandle;
    VkSurfaceKHR surfaceHandle = device.surface->getHandle();

    VkSurfaceCapabilitiesKHR capabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalHandle, surfaceHandle, &capabilities);

    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(physicalHandle, surfaceHandle, &formatCount, nullptr);
    std::vector<VkSurfaceFormatKHR> formats(formatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(physicalHandle, surfaceHandle, &formatCount, formats.data());

    uint32_t modeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(physicalHandle, surfaceHandle, &modeCount, nullptr);
    std::vector<VkPresentModeKHR> presentModes(modeCount);
    vkGetPhysicalDeviceSurfacePresentModesKHR(physicalHandle, surfaceHandle, &modeCount, presentModes.data());

    VkSurfaceFormatKHR surfaceFormat = formats[0];
    VkPresentModeKHR presentMode = presentModes[0];
    VkExtent2D extent = {1366, 768};

    uint32_t imageCount = capabilities.minImageCount + 1;
    if (capabilities.maxImageCount > 0 && capabilities.maxImageCount < imageCount)
        imageCount = capabilities.maxImageCount;

    VkSwapchainCreateInfoKHR createInfo = {
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .surface = surfaceHandle,
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
        .oldSwapchain = VK_NULL_HANDLE,
    };

    uint32_t queueFamilyIndices[] = {device.graphicsFamilyIndex.value(), device.presentFamilyIndex.value()};

    if (device.graphicsFamilyIndex.value() != device.presentFamilyIndex.value())
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

    if (vkCreateSwapchainKHR(*device.handle, &createInfo, nullptr, &handle) != VK_SUCCESS)
        throw std::exception("Failed to create swapchain");

    this->imageFormat = surfaceFormat.format;
    this->extent = extent;

    // swapchain images
    vkGetSwapchainImagesKHR(*device.handle, handle, &imageCount, nullptr);
    images.resize(imageCount);
    vkGetSwapchainImagesKHR(*device.handle, handle, &imageCount, images.data());

    frameInFlightCount = images.size();

    // image views

    imageViews.resize(imageCount);

    for (size_t i = 0; i < imageCount; ++i)
    {
        VkImageViewCreateInfo createInfo = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .image = images[i],
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .format = imageFormat,
            .components =
                {
                    .r = VK_COMPONENT_SWIZZLE_IDENTITY,
                    .g = VK_COMPONENT_SWIZZLE_IDENTITY,
                    .b = VK_COMPONENT_SWIZZLE_IDENTITY,
                    .a = VK_COMPONENT_SWIZZLE_IDENTITY,
                },
            .subresourceRange =
                {
                    .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                    .baseMipLevel = 0,
                    .levelCount = 1,
                    .baseArrayLayer = 0,
                    .layerCount = 1,
                },
        };

        VkResult res = vkCreateImageView(*device.handle, &createInfo, nullptr, &imageViews[i]);
        if (res != VK_SUCCESS)
            std::cerr << "Failed to create an image view : " << res << std::endl;
    }

    depthFormat = VK_FORMAT_D32_SFLOAT_S8_UINT;
    depthImage =
        std::make_unique<Image>(device, depthFormat, extent.width, extent.height, VK_IMAGE_TILING_OPTIMAL,
                                VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT);

    ImageLayoutTransitionBuilder iltb;
    ImageLayoutTransitionDirector iltd;
    iltd.createBuilder<VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL>(iltb);
    iltb.setImage(*depthImage);
    depthImage->transitionImageLayout(*iltb.build());

    depthImageView = depthImage->createImageView();
}

SwapChain::~SwapChain()
{
    vkDestroyImageView(*device.handle, depthImageView, nullptr);
    depthImage.reset();
    for (VkImageView &imageView : imageViews)
    {
        vkDestroyImageView(*device.handle, imageView, nullptr);
    }
    vkDestroySwapchainKHR(*device.handle, handle, nullptr);
}