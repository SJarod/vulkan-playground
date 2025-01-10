#include <iostream>

#include <window.hpp>

#include "graphics/surface.hpp"
#include "graphics/swapchain.hpp"

int WindowGLFW::init()
{
    return glfwInit();
}

void WindowGLFW::terminate()
{
    glfwTerminate();
}

WindowGLFW::WindowGLFW()
{
    m_handle = glfwCreateWindow(width, height, "test", NULL, NULL);
    if (!m_handle)
        throw;
}

WindowGLFW::~WindowGLFW()
{
    swapchain.reset();
    surface.reset();
}

void WindowGLFW::makeContextCurrent()
{
    glfwMakeContextCurrent(m_handle);
}

bool WindowGLFW::shouldClose()
{
    return glfwWindowShouldClose(m_handle);
}

void WindowGLFW::swapBuffers()
{
    glfwSwapBuffers(m_handle);
}

void WindowGLFW::pollEvents()
{
    glfwPollEvents();
}

VkResult WindowGLFW::createSurfacePredicate(VkInstance instance, const void *windowHandle,
                                            VkAllocationCallbacks *allocator, VkSurfaceKHR *surface)
{
    return glfwCreateWindowSurface(instance, (GLFWwindow *)windowHandle, allocator, surface);
}
