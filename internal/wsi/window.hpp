#pragma once

#include <memory>

#include <volk.h>
#include <GLFW/glfw3.h>


class Surface;
class SwapChain;

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

    inline GLFWwindow *getHandle() const
    {
        return m_handle;
    }

    static VkResult createSurfacePredicate(VkInstance instance, const void *windowHandle, VkAllocationCallbacks *allocator,
                                    VkSurfaceKHR *surface);
};