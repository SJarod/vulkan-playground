#pragma once

#include <glm/glm.hpp>

class UniformBufferObject
{
  public:
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 proj;

    static inline std::vector<VkDescriptorSetLayoutBinding> get_uniform_descriptor_set_layout_bindings()
    {
        std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings = {
            VkDescriptorSetLayoutBinding{
                .binding = 0,
                .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                .descriptorCount = 1,
                .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
                .pImmutableSamplers = nullptr,
            },
            VkDescriptorSetLayoutBinding{
                .binding = 1,
                .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                .descriptorCount = 1,
                .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
                .pImmutableSamplers = nullptr,
            }};
        return setLayoutBindings;
    }

    static inline std::vector<VkWriteDescriptorSet> get_uniform_descriptor_set_writes(
        VkDescriptorSet descriptorSet, const VkDescriptorBufferInfo &bufferInfo, const VkDescriptorImageInfo &imageInfo)
    {

        return std::vector<VkWriteDescriptorSet>{VkWriteDescriptorSet{
                                                     .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                                                     .dstSet = descriptorSet,
                                                     .dstBinding = 0,
                                                     .dstArrayElement = 0,
                                                     .descriptorCount = 1,
                                                     .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                                                     .pBufferInfo = &bufferInfo,
                                                     .pTexelBufferView = nullptr,
                                                 },
                                                 VkWriteDescriptorSet{
                                                     .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                                                     .dstSet = descriptorSet,
                                                     .dstBinding = 1,
                                                     .dstArrayElement = 0,
                                                     .descriptorCount = 1,
                                                     .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                                     .pImageInfo = &imageInfo,
                                                     .pTexelBufferView = nullptr,
                                                 }};
    }

    static inline std::vector<VkDescriptorPoolSize> get_uniform_descriptor_pool_sizes(uint32_t frameInFlightCount)
    {
        std::vector<VkDescriptorPoolSize> poolSizes = {VkDescriptorPoolSize{
                                                           .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                                                           .descriptorCount = frameInFlightCount,
                                                       },
                                                       VkDescriptorPoolSize{
                                                           .type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                                           .descriptorCount = frameInFlightCount,
                                                       }};
        return poolSizes;
    }
};
