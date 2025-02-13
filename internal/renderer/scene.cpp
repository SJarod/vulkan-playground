#include "mesh.hpp"
#include "texture.hpp"

#include "scene.hpp"

Scene::Scene(const std::weak_ptr<Device> device)
{
    m_objects.emplace_back(std::make_shared<Mesh>(device, "assets/viking_room.obj", "assets/viking_room.png"));

    const std::vector<Vertex> vertices = {{{-0.5f, -0.5f, 0.f}, {1.f, 0.f, 0.f, 1.f}, {1.f, 0.f}},
                                          {{0.5f, -0.5f, 0.f}, {0.f, 1.f, 0.f, 1.f}, {0.f, 0.f}},
                                          {{0.5f, 0.5f, 0.f}, {0.f, 0.f, 1.f, 1.f}, {0.f, 1.f}},
                                          {{-0.5f, 0.5f, 0.f}, {1.f, 1.f, 1.f, 1.f}, {1.f, 1.f}},
                                          {{-0.5f, -0.5f, -0.5f}, {1.f, 0.f, 0.f, 1.f}, {1.f, 0.f}},
                                          {{0.5f, -0.5f, -0.5f}, {0.f, 1.f, 0.f, 1.f}, {0.f, 0.f}},
                                          {{0.5f, 0.5f, -0.5f}, {0.f, 0.f, 1.f, 1.f}, {0.f, 1.f}},
                                          {{-0.5f, 0.5f, -0.5f}, {1.f, 1.f, 1.f, 1.f}, {1.f, 1.f}}};
    const std::vector<uint16_t> indices = {0, 1, 2, 2, 3, 0, 4, 5, 6, 6, 7, 4};
    std::shared_ptr<Mesh> mesh2 = std::make_shared<Mesh>(device, vertices, indices);
    const std::vector<unsigned char> imagePixels = {255, 0, 0, 255, 0, 255, 0, 255, 0, 0, 255, 255, 255, 0, 255, 255};
    mesh2->setTexture(std::move(std::make_unique<Texture>(device, 2, 2, imagePixels.data(), VK_FORMAT_R8G8B8A8_SRGB,
                                                          VK_IMAGE_TILING_OPTIMAL, VK_FILTER_NEAREST)));
    m_objects.push_back(mesh2);
}