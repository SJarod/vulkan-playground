#include <array>
#include <cassert>
#include <fstream>
#include <iostream>
#include <string>

#include "device.hpp"
#include "renderpass.hpp"

#include "engine/uniform.hpp"
#include "engine/vertex.hpp"

#include "pipeline.hpp"

VkShaderModule create_shader_module(VkDevice device, const std::vector<char> &code)
{
    VkShaderModuleCreateInfo createInfo = {
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .codeSize = code.size(),
        .pCode = reinterpret_cast<const uint32_t *>(code.data()),
    };

    VkShaderModule module;
    VkResult res = vkCreateShaderModule(device, &createInfo, nullptr, &module);
    if (res != VK_SUCCESS)
        std::cerr << "Failed to create shader module : " << res << std::endl;

    return module;
}
void destroy_shader_module(VkDevice device, VkShaderModule module)
{
    vkDestroyShaderModule(device, module, nullptr);
}

bool read_binary_file(const std::string &filename, std::vector<char> &out)
{
    std::ifstream file(filename, std::ios::ate | std::ios::binary);
    if (!file.is_open())
    {
        std::cerr << "Failed to open file : " << filename << std::endl;
        return false;
    }

    size_t fileSize = static_cast<size_t>(file.tellg());
    out.resize(fileSize);

    file.seekg(0);
    file.read(out.data(), fileSize);

    file.close();
    return true;
}

Pipeline::~Pipeline()
{
    if (!device.lock())
        return;

    auto deviceHandle = device.lock()->getHandle();

    vkDestroyPipelineLayout(deviceHandle, pipelineLayout, nullptr);
    vkDestroyDescriptorSetLayout(deviceHandle, descriptorSetLayout, nullptr);
    vkDestroyPipeline(deviceHandle, handle, nullptr);
}

void PipelineBuilder::restart()
{
    modules.clear();
    shaderStageCreateInfos.clear();
    dynamicStates.clear();

    pushConstantRanges.clear();

    product = std::unique_ptr<Pipeline>(new Pipeline);
}

void PipelineBuilder::addVertexShaderStage(const char *shaderName, const char *entryPoint)
{
    std::vector<char> shader;
    if (!read_binary_file("shaders/" + std::string(shaderName) + ".vert.spv", shader))
        return;

    modules.emplace_back(create_shader_module(device.lock()->getHandle(), shader));

    shaderStageCreateInfos.emplace_back(VkPipelineShaderStageCreateInfo{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .stage = VK_SHADER_STAGE_VERTEX_BIT,
        .module = modules[modules.size() - 1],
        .pName = entryPoint,
    });
}

void PipelineBuilder::addFragmentShaderStage(const char *shaderName, const char *entryPoint)
{
    std::vector<char> shader;
    if (!read_binary_file("shaders/" + std::string(shaderName) + ".frag.spv", shader))
        return;

    modules.emplace_back(create_shader_module(device.lock()->getHandle(), shader));

    shaderStageCreateInfos.emplace_back(VkPipelineShaderStageCreateInfo{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
        .module = modules[modules.size() - 1],
        .pName = entryPoint,
    });
}

void PipelineBuilder::addDynamicState(VkDynamicState state)
{
    dynamicStates.emplace_back(state);
}

void PipelineBuilder::setDrawTopology(VkPrimitiveTopology topology, bool bPrimitiveRestartEnable)
{
    this->topology = topology;
    this->bPrimitiveRestartEnable = bPrimitiveRestartEnable;
}

void PipelineBuilder::setExtent(VkExtent2D extent)
{
    this->extent = extent;
    product->extent = extent;
}

std::unique_ptr<Pipeline> PipelineBuilder::build()
{
    assert(device.lock());
    assert(renderPass);

    const VkDevice &deviceHandle = device.lock()->getHandle();

    VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
        .dynamicStateCount = static_cast<uint32_t>(dynamicStates.size()),
        .pDynamicStates = dynamicStates.data(),
    };

    // vertex buffer (enabling the binding for our Vertex structure)
    auto binding = Vertex::get_vertex_input_binding_description();
    auto attribs = Vertex::get_vertex_input_attribute_description();
    VkPipelineVertexInputStateCreateInfo vertexInputCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .vertexBindingDescriptionCount = 1,
        .pVertexBindingDescriptions = &binding,
        .vertexAttributeDescriptionCount = static_cast<uint32_t>(attribs.size()),
        .pVertexAttributeDescriptions = attribs.data(),
    };

    // draw mode
    VkPipelineInputAssemblyStateCreateInfo inputAssemblyCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        .topology = topology,
        .primitiveRestartEnable = static_cast<VkBool32>(bPrimitiveRestartEnable),
    };

    // viewport
    VkViewport viewport = {
        .x = 0.f,
        .y = 0.f,
        .width = static_cast<float>(extent.width),
        .height = static_cast<float>(extent.height),
        .minDepth = 0.f,
        .maxDepth = 1.f,
    };

    VkRect2D scissor = {
        .offset = {0, 0},
        .extent = extent,
    };

    VkPipelineViewportStateCreateInfo viewportStateCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        .viewportCount = 1,
        .scissorCount = 1,
    };

    // rasterizer
    VkPipelineRasterizationStateCreateInfo rasterizerCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        .depthClampEnable = depthClampEnable,
        .rasterizerDiscardEnable = rasterizerDiscardEnable,
        .polygonMode = polygonMode,
        .cullMode = cullMode,
        .frontFace = frontFace,
        .depthBiasEnable = depthBiasEnable,
        .depthBiasConstantFactor = depthBiasConstantFactor,
        .depthBiasClamp = depthBiasClamp,
        .depthBiasSlopeFactor = depthBiasSlopeFactor,
        .lineWidth = lineWidth,
    };

    // multisampling, anti-aliasing
    VkPipelineMultisampleStateCreateInfo multisamplingCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        .rasterizationSamples = rasterizationSamples,
        .sampleShadingEnable = sampleShadingEnable,
        .minSampleShading = minSampleShading,
        .pSampleMask = pSampleMask,
        .alphaToCoverageEnable = alphaToCoverageEnable,
        .alphaToOneEnable = alphaToOneEnable,
    };

    VkPipelineDepthStencilStateCreateInfo depthStencilCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
        .depthTestEnable = depthTestEnable,
        .depthWriteEnable = depthWriteEnable,
        .depthCompareOp = depthCompareOp,
        .depthBoundsTestEnable = depthBoundsTestEnable,
        .stencilTestEnable = stencilTestEnable,
        .front = front,
        .back = back,
        .minDepthBounds = minDepthBounds,
        .maxDepthBounds = maxDepthBounds,
    };

    // color blending
    VkPipelineColorBlendAttachmentState colorBlendAttachment = {
        .blendEnable = VK_TRUE,
        .srcColorBlendFactor = srcColorBlendFactor,
        .dstColorBlendFactor = dstColorBlendFactor,
        .colorBlendOp = colorBlendOp,
        .srcAlphaBlendFactor = srcAlphaBlendFactor,
        .dstAlphaBlendFactor = dstAlphaBlendFactor,
        .alphaBlendOp = alphaBlendOp,
        .colorWriteMask = colorWriteMask,
    };

    VkPipelineColorBlendStateCreateInfo colorBlendCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        .logicOpEnable = logicOpEnable,
        .logicOp = logicOp,
        .attachmentCount = 1,
        .pAttachments = &colorBlendAttachment,
        .blendConstants = {blendConstants[0], blendConstants[1], blendConstants[2], blendConstants[3]},
    };

    // descriptor set layout

    std::vector<VkDescriptorSetLayoutBinding> layoutBindings =
        UniformBufferObject::get_uniform_descriptor_set_layout_bindings();
    VkDescriptorSetLayoutCreateInfo createInfo = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .bindingCount = static_cast<uint32_t>(layoutBindings.size()),
        .pBindings = layoutBindings.data(),
    };
    VkResult res = vkCreateDescriptorSetLayout(deviceHandle, &createInfo, nullptr, &product->descriptorSetLayout);
    if (res != VK_SUCCESS)
    {
        std::cerr << "Failed to create descriptor set layout : " << res << std::endl;
        return nullptr;
    }
    std::vector<VkDescriptorSetLayout> setLayouts = {product->descriptorSetLayout};
    VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .setLayoutCount = static_cast<uint32_t>(setLayouts.size()),
        .pSetLayouts = setLayouts.data(),
        .pushConstantRangeCount = static_cast<uint32_t>(pushConstantRanges.size()),
        .pPushConstantRanges = pushConstantRanges.data(),
    };
    res = vkCreatePipelineLayout(deviceHandle, &pipelineLayoutCreateInfo, nullptr, &product->pipelineLayout);
    if (res != VK_SUCCESS)
    {
        std::cerr << "Failed to create pipeline layout : " << res << std::endl;
        return nullptr;
    }

    VkGraphicsPipelineCreateInfo pipelineCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        // shader stage
        .stageCount = static_cast<uint32_t>(shaderStageCreateInfos.size()),
        .pStages = shaderStageCreateInfos.data(),
        // fixed function stage
        .pVertexInputState = &vertexInputCreateInfo,
        .pInputAssemblyState = &inputAssemblyCreateInfo,
        .pViewportState = &viewportStateCreateInfo,
        .pRasterizationState = &rasterizerCreateInfo,
        .pMultisampleState = &multisamplingCreateInfo,
        .pDepthStencilState = &depthStencilCreateInfo,
        .pColorBlendState = &colorBlendCreateInfo,
        .pDynamicState = &dynamicStateCreateInfo,
        // pipeline layout
        .layout = product->pipelineLayout,
        // render pass
        .renderPass = renderPass->getHandle(),
        .subpass = 0,
        .basePipelineHandle = VK_NULL_HANDLE,
        .basePipelineIndex = -1,
    };

    res = vkCreateGraphicsPipelines(deviceHandle, VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr, &product->handle);
    if (res != VK_SUCCESS)
    {
        std::cerr << "Failed to create graphics pipeline : " << res << std::endl;
        return nullptr;
    }

    auto result = std::move(product);

    for (int i = 0; i < modules.size(); ++i)
    {
        destroy_shader_module(device.lock()->getHandle(), modules[i]);
    }

    return result;
}

void PipelineDirector::createColorDepthRasterizerBuilder(PipelineBuilder &builder)
{
    builder.addDynamicState(VK_DYNAMIC_STATE_VIEWPORT);
    builder.addDynamicState(VK_DYNAMIC_STATE_SCISSOR);

    builder.setDrawTopology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, VK_FALSE);

    builder.setDepthClampEnable(VK_FALSE);
    builder.setRasterizerDiscardEnable(VK_FALSE);
    builder.setPolygonMode(VK_POLYGON_MODE_FILL);
    builder.setCullMode(VK_CULL_MODE_BACK_BIT);
    builder.setFrontFace(VK_FRONT_FACE_COUNTER_CLOCKWISE);
    builder.setDepthBiasEnable(VK_FALSE);
    builder.setDepthBiasConstantFactor(0.f);
    builder.setDepthBiasClamp(0.f);
    builder.setDepthBiasSlopeFactor(0.f);
    builder.setLineWidth(1.f);

    builder.setRasterizationSamples(VK_SAMPLE_COUNT_1_BIT);
    builder.setSampleShadingEnable(VK_FALSE);
    builder.setMinSampleShading(1.f);
    builder.setPSampleMask(nullptr);
    builder.setAlphaToCoverageEnable(VK_FALSE);
    builder.setAlphaToOneEnable(VK_FALSE);

    builder.setDepthTestEnable(VK_TRUE);
    builder.setDepthWriteEnable(VK_TRUE);
    builder.setDepthCompareOp(VK_COMPARE_OP_LESS);
    builder.setDepthBoundsTestEnable(VK_FALSE);
    builder.setStencilTestEnable(VK_FALSE);
    builder.setFront({});
    builder.setBack({});
    builder.setMinDepthBounds(0.0f);
    builder.setMaxDepthBounds(1.0f);

    builder.setBlendEnable(VK_TRUE);
    builder.setSrcColorBlendFactor(VK_BLEND_FACTOR_SRC_ALPHA);
    builder.setDstColorBlendFactor(VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA);
    builder.setColorBlendOp(VK_BLEND_OP_ADD);
    builder.setSrcAlphaBlendFactor(VK_BLEND_FACTOR_ONE);
    builder.setDstAlphaBlendFactor(VK_BLEND_FACTOR_ZERO);
    builder.setAlphaBlendOp(VK_BLEND_OP_ADD);
    builder.setColorWriteMask(VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT |
                              VK_COLOR_COMPONENT_A_BIT);

    builder.setLogicOpEnable(VK_FALSE);
    builder.setLogicOp(VK_LOGIC_OP_COPY);
    builder.setBlendConstants(0.f, 0.f, 0.f, 0.f);
}

void Pipeline::recordBind(VkCommandBuffer &commandBuffer, uint32_t imageIndex)
{
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, handle);

    VkViewport viewport = {.x = 0.f,
                           .y = 0.f,
                           .width = static_cast<float>(extent.width),
                           .height = static_cast<float>(extent.height),
                           .minDepth = 0.f,
                           .maxDepth = 1.f};
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
    VkRect2D scissor = {.offset = {0, 0}, .extent = extent};
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
}
