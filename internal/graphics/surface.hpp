#pragma once

#include <vulkan/vulkan.h>

typedef VkResult (*PFN_CreateSurfacePredicate)(VkInstance, void *, VkAllocationCallbacks *, VkSurfaceKHR *);

class Context;

class Surface
{
  private:
    std::weak_ptr<Context> m_cx;

    VkSurfaceKHR m_handle;

  public:
    Surface(std::weak_ptr<Context> cx, PFN_CreateSurfacePredicate predicate, void *windowHandle);
    ~Surface();

    Surface(const Surface&) = delete;
    Surface& operator=(const Surface&) = delete;
    Surface(Surface&&) = delete;
    Surface& operator=(Surface&&) = delete;

    inline VkSurfaceKHR getHandle() const
    {
        return m_handle;
    }
};