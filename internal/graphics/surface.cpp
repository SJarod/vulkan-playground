#include "context.hpp"

#include "surface.hpp"

Surface::~Surface()
{
    vkDestroySurfaceKHR(cx.getInstanceHandle(), handle, nullptr);
}