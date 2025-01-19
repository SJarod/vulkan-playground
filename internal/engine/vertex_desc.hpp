#pragma once

#include <vulkan/vulkan.h>

#include <array>

#include "vertex.hpp"

struct VertexDescT
{
    // binding the data to the vertex shader
    static inline VkVertexInputBindingDescription get_vertex_binding_description()
    {
        // describe the buffer data
        VkVertexInputBindingDescription desc = {.binding = 0,
                                                .stride = sizeof(Vertex),
                                                // update every vertex (opposed to VK_VERTEX_INPUT_RATE_INSTANCE)
                                                .inputRate = VK_VERTEX_INPUT_RATE_VERTEX};

        return desc;
    }
    // 2 attributes descripction
    static inline std::array<VkVertexInputAttributeDescription, 2> get_vertex_attribute_description()
    {
        // attribute pointer
        std::array<VkVertexInputAttributeDescription, 2> desc;
        desc[0] = {
            .location = 0, .binding = 0, .format = VK_FORMAT_R32G32_SFLOAT, .offset = offsetof(Vertex, position)};
        desc[1] = {
            .location = 1, .binding = 0, .format = VK_FORMAT_R32G32B32A32_SFLOAT, .offset = offsetof(Vertex, color)};
        return desc;
    }
};