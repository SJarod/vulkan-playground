#include "mesh.hpp"

#include "scene.hpp"

Scene::Scene(const std::shared_ptr<Device> device)
{
    objects.emplace_back(std::make_unique<Mesh>(device, "assets/viking_room.obj", "assets/viking_room.png"));
}

Scene::~Scene()
{
}