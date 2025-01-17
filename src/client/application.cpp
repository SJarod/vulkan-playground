#include "graphics/context.hpp"
#include "graphics/device.hpp"
#include "renderer/renderer.hpp"
#include "wsi/window.hpp"

#include "application.hpp"

Application::Application()
{
    WindowGLFW::init();

    m_window = std::make_unique<WindowGLFW>();

    m_context = std::make_shared<Context>();
    m_context->addLayer("VK_LAYER_KHRONOS_validation");
    m_context->addInstanceExtension(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
    m_context->addInstanceExtension(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    auto requireExtensions = m_window->getRequiredExtensions();
    for (const auto &extension : requireExtensions)
    {
        m_context->addInstanceExtension(extension);
    }
    m_context->addDeviceExtension(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
    m_context->finishCreateContext();

    m_window->surface =
        std::make_unique<Surface>(*m_context, &WindowGLFW::createSurfacePredicate, m_window->getHandle());

    auto physicalDevices = m_context->getAvvailablePhysicalDevices();
    for (auto physicalDevice : physicalDevices)
    {
        m_devices.emplace_back(std::make_shared<Device>(*m_context, physicalDevice, m_window->surface.get()));
        (*(m_devices.end() - 1))->initLogicalDevice();
    }

    std::shared_ptr<Device> mainDevice = m_devices[0];

    m_window->swapchain = std::make_unique<SwapChain>(*mainDevice);

    m_renderer = std::make_shared<Renderer>(*mainDevice, *m_window->swapchain);
}

Application::~Application()
{
    m_renderer.reset();

    m_window.reset();

    m_devices.clear();
    m_context.reset();

    WindowGLFW::terminate();
}

void Application::run()
{
    m_window->makeContextCurrent();

    std::shared_ptr<Device> mainDevice = m_devices[0];

    while (!m_window->shouldClose())
    {
        m_window->swapBuffers();
        m_window->pollEvents();
    }
}