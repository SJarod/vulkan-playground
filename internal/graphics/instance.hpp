#pragma once

#include <iostream>
#include <memory>

#include <vulkan/vulkan.h>

class Context;

class Instance
{
  private:
    std::unique_ptr<VkInstance> m_handle;

#if 0
    static VKAPI_ATTR VkBool32 VKAPI_CALL debugReportCallback(VkDebugReportFlagsEXT flags,
                                                              VkDebugReportObjectTypeEXT objectType, uint64_t object,
                                                              size_t location, int32_t messageCode,
                                                              const char *pLayerPrefix, const char *pMessage,
                                                              void *pUserData)
    {

        std::cerr << "debug report: " << pMessage << std::endl;

        return VK_FALSE;
    }
#endif

  public:
    Instance(const Context &cx);
    ~Instance();

  public:
    inline VkInstance getHandle() const
    {
        return *m_handle;
    }
};