#pragma once

#include <vulkan/vulkan.h>


typedef VkResult (*PFN_CreateSurfacePredicate)(VkInstance, void *, VkAllocationCallbacks *, VkSurfaceKHR *);

class Context;

class Surface
{
  private:
    const Context &cx;

  private:
    VkSurfaceKHR handle;

  public:
    Surface(const Context &cx, PFN_CreateSurfacePredicate predicate, void* windowHandle);
    ~Surface();

    inline VkSurfaceKHR getHandle() const
    {
        return handle;
    }
};