#include "graphics/context.hpp"
#include "graphics/device.hpp"
#include "renderer/renderer.hpp"
#include "wsi/window.hpp"

#include "engine/vertex.hpp"

#include "renderer/mesh.hpp"

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

    auto physicalDevices = m_context->getAvailablePhysicalDevices();
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

void Application::runLoop()
{
    std::shared_ptr<Device> mainDevice = m_devices[0];

    m_window->makeContextCurrent();

    const std::vector<Vertex> vertices = {{{-0.5f, -0.5f, 0.f}, {1.f, 0.f, 0.f, 1.f}, {1.f, 0.f}},
                                          {{0.5f, -0.5f, 0.f}, {0.f, 1.f, 0.f, 1.f}, {0.f, 0.f}},
                                          {{0.5f, 0.5f, 0.f}, {0.f, 0.f, 1.f, 1.f}, {0.f, 1.f}},
                                          {{-0.5f, 0.5f, 0.f}, {1.f, 1.f, 1.f, 1.f}, {1.f, 1.f}}};
    const std::vector<uint16_t> indices = {0, 1, 2, 2, 3, 0};
    std::unique_ptr<Mesh> triangleMesh = std::make_unique<Mesh>(*mainDevice, vertices, indices);

    m_renderer->writeDescriptorSets();

    while (!m_window->shouldClose())
    {
        m_window->pollEvents();

        uint32_t imageIndex = m_renderer->acquireBackBuffer();

        m_renderer->updateUniformBuffers(imageIndex);

        m_renderer->recordBackBufferBeginRenderPass(imageIndex);
        m_renderer->recordBackBufferDescriptorSetsCommands(imageIndex);
        m_renderer->recordBackBufferDrawObjectCommands(*triangleMesh);
        m_renderer->recordBackBufferEndRenderPass();

        m_renderer->submitBackBuffer();
        m_renderer->presentBackBuffer(imageIndex);

        m_renderer->swapBuffers();

        m_window->swapBuffers();

        // static int a = 0;
        // if (a == 2)
        //     break;
        // ++a;
    }
}