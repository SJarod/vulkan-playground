#include <assimp/Importer.hpp>

#include "graphics/context.hpp"
#include "graphics/device.hpp"
#include "renderer/renderer.hpp"
#include "wsi/window.hpp"

#include "engine/vertex.hpp"

#include "renderer/mesh.hpp"
#include "renderer/scene.hpp"
#include "renderer/texture.hpp"

#include "engine/camera.hpp"

#include "application.hpp"

Application::Application()
{
    WindowGLFW::init();

    m_window = std::make_unique<WindowGLFW>();

    m_context = std::make_shared<Context>();
    m_context->addLayer("VK_LAYER_KHRONOS_validation");
    m_context->addLayer("VK_LAYER_LUNARG_monitor");
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
        m_devices.emplace_back(std::make_shared<Device>(m_context, physicalDevice, m_window->surface.get()));
        (*(m_devices.end() - 1))->initLogicalDevice();
    }

    std::shared_ptr<Device> mainDevice = m_devices[0];

    m_window->swapchain = std::make_shared<SwapChain>(*mainDevice);

    RendererBuilder rb;
    rb.setDevice(mainDevice);
    rb.setSwapChain(m_window->swapchain);
    m_renderer = rb.build();
}

Application::~Application()
{
    m_renderer.reset();
    m_scene.reset();

    m_window.reset();

    m_devices.clear();
    m_context.reset();

    WindowGLFW::terminate();
}

void Application::runLoop()
{
    std::shared_ptr<Device> mainDevice = m_devices[0];

    m_window->makeContextCurrent();

    m_scene = std::make_unique<Scene>(mainDevice);
    for (int i = 0; i < m_scene->objects.size(); ++i)
    {
        m_renderer->registerRenderState(m_scene->objects[i]);
    }

    Camera camera;

    std::pair<double, double> mousePos;
    glfwGetCursorPos(m_window->getHandle(), &mousePos.first, &mousePos.second);
    glfwSetInputMode(m_window->getHandle(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    while (!m_window->shouldClose())
    {
        m_timeManager.markFrame();
        float deltaTime = m_timeManager.deltaTime();

        double xpos, ypos;
        glfwGetCursorPos(m_window->getHandle(), &xpos, &ypos);
        std::pair<double, double> deltaMousePos;
        deltaMousePos.first = mousePos.first - xpos;
        deltaMousePos.second = mousePos.second - ypos;
        mousePos.first = xpos;
        mousePos.second = ypos;

        m_window->pollEvents();

        float pitch = (float)deltaMousePos.second * camera.sensitivity * deltaTime;
        float yaw = (float)deltaMousePos.first * camera.sensitivity * deltaTime;
        camera.transform.rotation =
            glm::quat(glm::vec3(-pitch, 0.f, 0.f)) * camera.transform.rotation * glm::quat(glm::vec3(0.f, -yaw, 0.f));

        float xaxisInput = (glfwGetKey(m_window->getHandle(), GLFW_KEY_A) == GLFW_PRESS) -
                           (glfwGetKey(m_window->getHandle(), GLFW_KEY_D) == GLFW_PRESS);
        float zaxisInput = (glfwGetKey(m_window->getHandle(), GLFW_KEY_W) == GLFW_PRESS) -
                           (glfwGetKey(m_window->getHandle(), GLFW_KEY_S) == GLFW_PRESS);
        float yaxisInput = (glfwGetKey(m_window->getHandle(), GLFW_KEY_Q) == GLFW_PRESS) -
                           (glfwGetKey(m_window->getHandle(), GLFW_KEY_E) == GLFW_PRESS);
        glm::vec3 dir = glm::vec3(xaxisInput, yaxisInput, zaxisInput) * glm::mat3_cast(camera.transform.rotation);
        if (!(xaxisInput == 0.f && zaxisInput == 0.f && yaxisInput == 0.f))
            dir = glm::normalize(dir);
        camera.transform.position += camera.speed * dir * deltaTime;

        uint32_t imageIndex = m_renderer->acquireBackBuffer();

        m_renderer->recordRenderers(imageIndex, camera);

        m_renderer->submitBackBuffer();
        m_renderer->presentBackBuffer(imageIndex);

        m_renderer->swapBuffers();

        m_window->swapBuffers();
    }
}