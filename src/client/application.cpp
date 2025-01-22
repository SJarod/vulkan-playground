#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>

#include "graphics/context.hpp"
#include "graphics/device.hpp"
#include "renderer/renderer.hpp"
#include "wsi/window.hpp"

#include "engine/vertex.hpp"

#include "renderer/mesh.hpp"
#include "renderer/texture.hpp"

#include "engine/camera.hpp"

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

    Assimp::Importer importer;
    const aiScene *pScene =
        importer.ReadFile("assets/viking_room.obj", aiProcess_Triangulate | aiProcess_GenSmoothNormals |
                                                        aiProcess_FlipUVs | aiProcess_JoinIdenticalVertices);
    std::unique_ptr<Mesh> triangleMesh = std::make_unique<Mesh>(*mainDevice, pScene);

    const std::vector<unsigned char> imagePixels = {255, 0, 0, 255, 0, 255, 0, 255, 0, 0, 255, 255, 255, 0, 255, 255};
    std::unique_ptr<Texture> simpleTexture = std::make_unique<Texture>(
        *mainDevice, 2, 2, imagePixels.data(), VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_FILTER_NEAREST);

    m_renderer->writeDescriptorSets(*simpleTexture);

    Camera camera;

    while (!m_window->shouldClose())
    {
        m_window->pollEvents();

        double xpos, ypos;
        glfwGetCursorPos(m_window->getHandle(), &xpos, &ypos);
        float pitch = (float)ypos * camera.sensitivity;
        float yaw = (float)xpos * camera.sensitivity;
        camera.transform.rotation = glm::quat(glm::vec3(-pitch, 0.f, 0.f)) * glm::quat(glm::vec3(0.f, -yaw, 0.f));
        float xaxisInput = (glfwGetKey(m_window->getHandle(), GLFW_KEY_A) == GLFW_PRESS) -
                           (glfwGetKey(m_window->getHandle(), GLFW_KEY_D) == GLFW_PRESS);
        float zaxisInput = (glfwGetKey(m_window->getHandle(), GLFW_KEY_W) == GLFW_PRESS) -
                           (glfwGetKey(m_window->getHandle(), GLFW_KEY_S) == GLFW_PRESS);
        float yaxisInput = (glfwGetKey(m_window->getHandle(), GLFW_KEY_E) == GLFW_PRESS) -
                           (glfwGetKey(m_window->getHandle(), GLFW_KEY_Q) == GLFW_PRESS);
        glm::vec3 dir = glm::vec3(xaxisInput, yaxisInput, zaxisInput) * glm::mat3_cast(camera.transform.rotation);
        if (!(xaxisInput == 0.f && zaxisInput == 0.f && yaxisInput == 0.f))
            dir = glm::normalize(dir);
        camera.transform.position += 0.2f * dir;

        uint32_t imageIndex = m_renderer->acquireBackBuffer();

        m_renderer->updateUniformBuffers(imageIndex, camera);

        m_renderer->recordBackBufferBeginRenderPass(imageIndex);
        m_renderer->recordBackBufferDescriptorSetsCommands(imageIndex);
        m_renderer->recordBackBufferDrawObjectCommands(*triangleMesh);
        m_renderer->recordBackBufferEndRenderPass();

        m_renderer->submitBackBuffer();
        m_renderer->presentBackBuffer(imageIndex);

        m_renderer->swapBuffers();

        m_window->swapBuffers();
    }
}