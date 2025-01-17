#pragma once

#include <memory>
#include <vector>

class WindowGLFW;
class Context;
class Device;
class Renderer;

class Application
{
  private:
    std::unique_ptr<WindowGLFW> m_window;

    std::shared_ptr<Context> m_context;
    std::vector<std::shared_ptr<Device>> m_devices;

    std::shared_ptr<Renderer> m_renderer;

  public:
    Application();
    ~Application();

    void run();
};