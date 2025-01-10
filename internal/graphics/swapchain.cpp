#include "swapchain.cpp"

#include "device.hpp"

SwapChain::~SwapChain()
{
    vkDestroySwapchainKHR(device.handle, handle, nullptr);
}