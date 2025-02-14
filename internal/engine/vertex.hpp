#pragma once

#include <array>

#include <glm/glm.hpp>

#include <vulkan/vulkan.h>

class Vertex
{
  public:
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec4 color;
    glm::vec2 uv;

    static inline VkVertexInputBindingDescription get_vertex_input_binding_description()
    {
        // describe the buffer data
        VkVertexInputBindingDescription desc = {
            .binding = 0,
            .stride = sizeof(Vertex),
            // update every vertex (opposed to VK_VERTEX_INPUT_RATE_INSTANCE)
            .inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
        };

        return desc;
    }
    static inline std::array<VkVertexInputAttributeDescription, 4> get_vertex_input_attribute_description()
    {
        // attribute pointer
        std::array<VkVertexInputAttributeDescription, 4> desc;
        desc[0] = {
            .location = 0,
            .binding = 0,
            .format = VK_FORMAT_R32G32B32_SFLOAT,
            .offset = offsetof(Vertex, position),
        };
        desc[1] = {
            .location = 1,
            .binding = 0,
            .format = VK_FORMAT_R32G32B32_SFLOAT,
            .offset = offsetof(Vertex, normal),
        };
        desc[2] = {
            .location = 2,
            .binding = 0,
            .format = VK_FORMAT_R32G32B32A32_SFLOAT,
            .offset = offsetof(Vertex, color),
        };
        desc[3] = {
            .location = 3,
            .binding = 0,
            .format = VK_FORMAT_R32G32_SFLOAT,
            .offset = offsetof(Vertex, uv),
        };
        return desc;
    }
};