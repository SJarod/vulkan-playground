#pragma once

#include <memory>
#include <vector>

class WindowGLFW;
class Context;
class Device;

class Application
{
private:
    std::unique_ptr<WindowGLFW> m_window;

    std::shared_ptr<Context> m_context;
    std::vector<std::shared_ptr<Device>> devices;

public:
    Application();
    ~Application();

    void run();
};