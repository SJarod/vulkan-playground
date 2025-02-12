#include <glm/glm.hpp>
#include <iostream>

#include "engine/camera.hpp"
#include "engine/uniform.hpp"
#include "graphics/buffer.hpp"
#include "graphics/device.hpp"
#include "graphics/pipeline.hpp"
#include "graphics/renderpass.hpp"
#include "mesh.hpp"
#include "texture.hpp"

#include "renderstate.hpp"

RenderState::~RenderState()
{
    if (!device.lock())
        return;

    vkDestroyDescriptorPool(*device.lock()->handle, descriptorPool, nullptr);

    pipeline.reset();
}

void RenderState::updateUniformBuffers(uint32_t imageIndex, const Camera &camera)
{
    UniformBufferObject ubo = {
        .model = glm::identity<glm::mat4>(),
        .view = camera.getViewMatrix(),
        .proj = camera.proj,
    };
    memcpy(uniformBuffersMapped[imageIndex], &ubo, sizeof(ubo));
}

void RenderState::recordBackBufferDescriptorSetsCommands(VkCommandBuffer &commandBuffer, uint32_t imageIndex)
{
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->getPipelineLayout(), 0, 1,
                            &descriptorSets[imageIndex], 0, nullptr);
}

void MeshRenderStateBuilder::setPipeline(std::shared_ptr<Pipeline> pipeline)
{
    product->pipeline = pipeline;
}
void MeshRenderStateBuilder::addPoolSize(VkDescriptorType poolSizeType)
{
    this->poolSizes.push_back(VkDescriptorPoolSize{
        .type = poolSizeType,
        .descriptorCount = frameInFlightCount,
    });
}

std::unique_ptr<RenderState> MeshRenderStateBuilder::build()
{
    assert(device.lock());

    const VkDevice &deviceHandle = device.lock()->getHandle();

    // descriptor pool
    std::vector<VkDescriptorPoolSize> poolSizes = {};
    VkDescriptorPoolCreateInfo createInfo = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .maxSets = frameInFlightCount,
        .poolSizeCount = static_cast<uint32_t>(poolSizes.size()),
        .pPoolSizes = poolSizes.data(),
    };
    VkResult res = vkCreateDescriptorPool(deviceHandle, &createInfo, nullptr, &product->descriptorPool);
    if (res != VK_SUCCESS)
    {
        std::cerr << "Failed to create descriptor pool : " << res << std::endl;
        return nullptr;
    }

    // descriptor set
    std::vector<VkDescriptorSetLayout> setLayouts(frameInFlightCount, product->pipeline->getDescriptorSetLayout());
    VkDescriptorSetAllocateInfo descriptorSetAllocInfo = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .descriptorPool = product->descriptorPool,
        .descriptorSetCount = frameInFlightCount,
        .pSetLayouts = setLayouts.data(),
    };
    product->descriptorSets.resize(frameInFlightCount);
    res = vkAllocateDescriptorSets(deviceHandle, &descriptorSetAllocInfo, product->descriptorSets.data());
    if (res != VK_SUCCESS)
    {
        std::cerr << "Failed to allocate descriptor sets : " << res << std::endl;
        return nullptr;
    }

    // uniform buffers

    product->uniformBuffers.resize(frameInFlightCount);
    product->uniformBuffersMapped.resize(frameInFlightCount);
    for (int i = 0; i < product->uniformBuffers.size(); ++i)
    {
        product->uniformBuffers[i] =
            std::make_unique<Buffer>(*device.lock(), sizeof(UniformBufferObject), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        vkMapMemory(deviceHandle, product->uniformBuffers[i]->memory, 0, sizeof(UniformBufferObject), 0,
                    &product->uniformBuffersMapped[i]);
    }

    for (int i = 0; i < product->descriptorSets.size(); ++i)
    {
        VkDescriptorBufferInfo bufferInfo = {
            .buffer = product->uniformBuffers[i]->handle,
            .offset = 0,
            .range = sizeof(UniformBufferObject),
        };
        VkDescriptorImageInfo imageInfo = {
            .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        };
        if (texture)
        {
            imageInfo.sampler = texture->sampler;
            imageInfo.imageView = texture->imageView;
        }
        std::vector<VkWriteDescriptorSet> writes =
            UniformBufferObject::get_uniform_descriptor_set_writes(product->descriptorSets[i], bufferInfo, imageInfo);
        vkUpdateDescriptorSets(deviceHandle, static_cast<uint32_t>(writes.size()), writes.data(), 0, nullptr);
    }

    auto result = std::move(product);
    return result;
}

void RenderStateDirector::createUniformAndSamplerRenderStateBuilder(RenderStateBuilderI &builder)
{
    builder.addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
    builder.addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
}

void MeshRenderState::recordBackBufferDrawObjectCommands(VkCommandBuffer &commandBuffer)
{
    auto meshPtr = mesh.lock();

    VkBuffer vbos[] = {meshPtr->vertexBuffer->handle};
    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, vbos, offsets);
    vkCmdBindIndexBuffer(commandBuffer, meshPtr->indexBuffer->handle, 0, VK_INDEX_TYPE_UINT16);
    vkCmdDrawIndexed(commandBuffer, meshPtr->indices.size(), 1, 0, 0, 0);
}
