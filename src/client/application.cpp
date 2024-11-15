#include "application.hpp"

Application::Application()
{
    m_context = std::make_shared<Context>();
}

void Application::run()
{
    WindowGLFW::init();

    m_window = std::make_unique<WindowGLFW>();

    m_window->makeContextCurrent();
    while (!m_window->shouldClose())
    {
        m_window->swapBuffers();
        m_window->pollEvents();
    }

    WindowGLFW::terminate();
}