#include "wsi/window.hpp"
#include "graphics/context.hpp"
#include "graphics/device.hpp"

#include "application.hpp"


Application::Application()
{
    m_context = std::make_shared<Context>();
    // m_context->addLayer("VK_LAYER_KHRONOS_validation");
    m_context->addDeviceExtension(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
    m_context->finishCreateContext();

    auto physicalDevices = m_context->getAvvailablePhysicalDevices();
    for (auto physicalDevice : physicalDevices)
    {
        devices.emplace_back(std::make_shared<Device>(*m_context, physicalDevice));
        (*(devices.end() - 1))->initLogicalDevice();
    }
}

Application::~Application()
{
    devices.clear();
    m_context.reset();
    m_window.reset();
}

void Application::run()
{
    std::shared_ptr<Device> mainDevice = devices[0];

    WindowGLFW::init();

    m_window = std::make_unique<WindowGLFW>();
    m_window->surface = mainDevice->createSurface(&WindowGLFW::createSurfacePredicate, m_window->getHandle());

    m_window->makeContextCurrent();
    while (!m_window->shouldClose())
    {
        m_window->swapBuffers();
        m_window->pollEvents();
    }

    WindowGLFW::terminate();
}