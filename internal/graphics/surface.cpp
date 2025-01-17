#include "context.hpp"

#include "surface.hpp"

Surface::Surface(const Context &cx, PFN_CreateSurfacePredicate predicate, void *windowHandle) : cx(cx), handle(handle)
{
    VkResult result = predicate(cx.getInstanceHandle(), windowHandle, nullptr, &handle);
    if (result != VK_SUCCESS)
        std::cerr << "Failed to create window surface : " << result << std::endl;
}

Surface::~Surface()
{
    if (handle)
        vkDestroySurfaceKHR(cx.getInstanceHandle(), handle, nullptr);
}