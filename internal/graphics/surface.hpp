#pragma once

#include <volk.h>

class Surface
{
  private:
    VkSurfaceKHR handle;

  public:
    inline VkSurfaceKHR getHandle() const
    {
        return handle;
    }
};