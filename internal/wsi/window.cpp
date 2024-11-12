#include <window.hpp>

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

void WindowGLFW::makeContextCurrent()
{
    glfwMakeContextCurrent(m_handle);
}

bool WindowGLFW::windowShouldClose()
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