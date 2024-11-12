#pragma once

#include <memory>

#include <wsi/window.hpp>
#include <graphics/context.hpp>

class Application
{
private:
    std::unique_ptr<WindowGLFW> m_window;

    std::shared_ptr<Context> m_context;

public:
    Application();

    void run();
};