#pragma once

#include <volk.h>

class Context;

class Surface
{
  private:
    const Context &cx;

  private:
    VkSurfaceKHR handle;

  public:
    Surface(const Context &cx, VkSurfaceKHR handle) : cx(cx), handle(handle)
    {
    }
    ~Surface();

    inline VkSurfaceKHR getHandle() const
    {
        return handle;
    }
};