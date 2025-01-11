#include "swapchain.hpp"

#include "device.hpp"

SwapChain::~SwapChain()
{
    vkDestroySwapchainKHR(*device.handle, handle, nullptr);
}