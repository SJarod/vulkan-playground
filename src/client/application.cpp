#include "graphics/context.hpp"
#include "graphics/device.hpp"
#include "wsi/window.hpp"

#include "application.hpp"

Application::Application()
{
    WindowGLFW::init();

    m_window = std::make_unique<WindowGLFW>();

    m_context = std::make_shared<Context>();
    m_context->addLayer("VK_LAYER_KHRONOS_validation");
#if 0
    m_context->addInstanceExtension(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
    m_context->addInstanceExtension(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif
    auto requireExtensions = m_window->getRequiredExtensions();
    for (const auto &extension : requireExtensions)
    {
        m_context->addInstanceExtension(extension);
    }
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
    m_window.reset();
    
    m_context.reset();

    WindowGLFW::terminate();
}

void Application::run()
{
    m_window->makeContextCurrent();

    std::shared_ptr<Device> mainDevice = devices[0];

    m_window->surface = mainDevice->createSurface(&WindowGLFW::createSurfacePredicate, m_window->getHandle());

    while (!m_window->shouldClose())
    {
        m_window->swapBuffers();
        m_window->pollEvents();
    }
}