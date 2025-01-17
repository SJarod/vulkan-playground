#pragma once

#include <memory>
#include <vector>

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>

#include "graphics/surface.hpp"
#include "graphics/swapchain.hpp"

class WindowI
{
};

class WindowGLFW : public WindowI
{
  private:
    GLFWwindow *m_handle;

    int width = 1366;
    int height = 768;

  public:
    std::unique_ptr<Surface> surface;
    std::unique_ptr<SwapChain> swapchain;

  public:
    static int init();
    static void terminate();

  public:
    WindowGLFW();
    ~WindowGLFW();

    void makeContextCurrent();
    bool shouldClose();

    void swapBuffers();
    void pollEvents();

    const std::vector<const char *> getRequiredExtensions() const;

    inline GLFWwindow *getHandle() const
    {
        return m_handle;
    }

    static VkResult createSurfacePredicate(VkInstance instance, void *windowHandle, VkAllocationCallbacks *allocator,
                                           VkSurfaceKHR *surface);
};