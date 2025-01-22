#include <array>
#include <fstream>
#include <iostream>
#include <string>

#include "device.hpp"
#include "renderpass.hpp"

#include "engine/uniform.hpp"
#include "engine/vertex.hpp"

#include "pipeline.hpp"

VkShaderModule Pipeline::create_shader_module(VkDevice device, const std::vector<char> &code)
{
    VkShaderModuleCreateInfo createInfo = {.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
                                           .codeSize = code.size(),
                                           .pCode = reinterpret_cast<const uint32_t *>(code.data())};

    VkShaderModule module;
    VkResult res = vkCreateShaderModule(device, &createInfo, nullptr, &module);
    if (res != VK_SUCCESS)
        std::cerr << "Failed to create shader module : " << res << std::endl;

    return module;
}
void Pipeline::destroy_shader_module(VkDevice device, VkShaderModule module)
{
    vkDestroyShaderModule(device, module, nullptr);
}

bool Pipeline::read_binary_file(const std::string &filename, std::vector<char> &out)
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

Pipeline::Pipeline(const Device &device, const char *shaderName, const RenderPass &renderPass, const VkExtent2D &extent)
    : device(device)
{
    std::vector<char> vs;
    if (!read_binary_file("shaders/" + std::string(shaderName) + ".vert.spv", vs))
        return;
    std::vector<char> fs;
    if (!read_binary_file("shaders/" + std::string(shaderName) + ".frag.spv", fs))
        return;

    VkShaderModule vsModule = create_shader_module(*device.handle, vs);
    VkShaderModule fsModule = create_shader_module(*device.handle, fs);

    VkPipelineShaderStageCreateInfo vsStageCreateInfo = {.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                                                         .stage = VK_SHADER_STAGE_VERTEX_BIT,
                                                         .module = vsModule,
                                                         .pName = "main",
                                                         // for shader constants values
                                                         .pSpecializationInfo = nullptr};

    VkPipelineShaderStageCreateInfo fsStageCreateInfo = {.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                                                         .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
                                                         .module = fsModule,
                                                         .pName = "main",
                                                         .pSpecializationInfo = nullptr};

    VkPipelineShaderStageCreateInfo shaderStagesCreateInfo[] = {vsStageCreateInfo, fsStageCreateInfo};

    std::vector<VkDynamicState> dynamicStates = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};

    VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
        .dynamicStateCount = static_cast<uint32_t>(dynamicStates.size()),
        .pDynamicStates = dynamicStates.data()};

    // vertex buffer (enabling the binding for our Vertex structure)
    auto binding = Vertex::get_vertex_input_binding_description();
    auto attribs = Vertex::get_vertex_input_attribute_description();
    VkPipelineVertexInputStateCreateInfo vertexInputCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .vertexBindingDescriptionCount = 1,
        .pVertexBindingDescriptions = &binding,
        .vertexAttributeDescriptionCount = static_cast<uint32_t>(attribs.size()),
        .pVertexAttributeDescriptions = attribs.data()};

    // draw mode
    VkPipelineInputAssemblyStateCreateInfo inputAssemblyCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
        .primitiveRestartEnable = VK_FALSE};

    // viewport
    VkViewport viewport = {.x = 0.f,
                           .y = 0.f,
                           .width = static_cast<float>(extent.width),
                           .height = static_cast<float>(extent.height),
                           .minDepth = 0.f,
                           .maxDepth = 1.f};

    VkRect2D scissor = {.offset = {0, 0}, .extent = extent};

    VkPipelineViewportStateCreateInfo viewportStateCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO, .viewportCount = 1, .scissorCount = 1};

    // rasterizer
    VkPipelineRasterizationStateCreateInfo rasterizerCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        .depthClampEnable = VK_FALSE,
        .rasterizerDiscardEnable = VK_FALSE,
        .polygonMode = VK_POLYGON_MODE_FILL,
        .cullMode = VK_CULL_MODE_BACK_BIT,
        .frontFace = VK_FRONT_FACE_CLOCKWISE,
        .depthBiasEnable = VK_FALSE,
        .depthBiasConstantFactor = 0.f,
        .depthBiasClamp = 0.f,
        .depthBiasSlopeFactor = 0.f,
        .lineWidth = 1.f};

    // multisampling, anti-aliasing
    VkPipelineMultisampleStateCreateInfo multisamplingCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
        .sampleShadingEnable = VK_FALSE,
        .minSampleShading = 1.f,
        .pSampleMask = nullptr,
        .alphaToCoverageEnable = VK_FALSE,
        .alphaToOneEnable = VK_FALSE};

    // color blending
    VkPipelineColorBlendAttachmentState colorBlendAttachment = {
        .blendEnable = VK_TRUE,
        .srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA,
        .dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
        .colorBlendOp = VK_BLEND_OP_ADD,
        .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
        .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
        .alphaBlendOp = VK_BLEND_OP_ADD,
        .colorWriteMask =
            VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT};

    VkPipelineColorBlendStateCreateInfo colorBlendCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        .logicOpEnable = VK_FALSE,
        .logicOp = VK_LOGIC_OP_COPY,
        .attachmentCount = 1,
        .pAttachments = &colorBlendAttachment,
        .blendConstants = {0.f, 0.f, 0.f, 0.f}};

    // descriptor set layout

    std::vector<VkDescriptorSetLayoutBinding> layoutBindings =
        UniformBufferObject::get_uniform_descriptor_set_layout_bindings();
    VkDescriptorSetLayoutCreateInfo createInfo = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .bindingCount = static_cast<uint32_t>(layoutBindings.size()),
        .pBindings = layoutBindings.data(),
    };
    VkResult res = vkCreateDescriptorSetLayout(*device.handle, &createInfo, nullptr, &descriptorSetLayout);
    if (res != VK_SUCCESS)
    {
        std::cerr << "Failed to create descriptor set layout : " << res << std::endl;
        return;
    }
    std::vector<VkDescriptorSetLayout> setLayouts = {descriptorSetLayout};
    VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
                                                           .setLayoutCount = static_cast<uint32_t>(setLayouts.size()),
                                                           .pSetLayouts = setLayouts.data(),
                                                           .pushConstantRangeCount = 0,
                                                           .pPushConstantRanges = nullptr};
    res = vkCreatePipelineLayout(*device.handle, &pipelineLayoutCreateInfo, nullptr, &pipelineLayout);
    if (res != VK_SUCCESS)
    {
        std::cerr << "Failed to create pipeline layout : " << res << std::endl;
        return;
    }

    VkGraphicsPipelineCreateInfo pipelineCreateInfo = {.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
                                                       // shader stage
                                                       .stageCount = 2,
                                                       .pStages = shaderStagesCreateInfo,
                                                       // fixed function stage
                                                       .pVertexInputState = &vertexInputCreateInfo,
                                                       .pInputAssemblyState = &inputAssemblyCreateInfo,
                                                       .pViewportState = &viewportStateCreateInfo,
                                                       .pRasterizationState = &rasterizerCreateInfo,
                                                       .pMultisampleState = &multisamplingCreateInfo,
                                                       .pDepthStencilState = nullptr,
                                                       .pColorBlendState = &colorBlendCreateInfo,
                                                       .pDynamicState = &dynamicStateCreateInfo,
                                                       // pipeline layout
                                                       .layout = pipelineLayout,
                                                       // render pass
                                                       .renderPass = renderPass.handle,
                                                       .subpass = 0,
                                                       .basePipelineHandle = VK_NULL_HANDLE,
                                                       .basePipelineIndex = -1};

    res = vkCreateGraphicsPipelines(*device.handle, VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr, &handle);
    if (res != VK_SUCCESS)
    {
        std::cerr << "Failed to create graphics pipeline : " << res << std::endl;
        return;
    }

    destroy_shader_module(*device.handle, vsModule);
    destroy_shader_module(*device.handle, fsModule);
}

Pipeline::~Pipeline()
{
    vkDestroyPipelineLayout(*device.handle, pipelineLayout, nullptr);
    vkDestroyDescriptorSetLayout(*device.handle, descriptorSetLayout, nullptr);
    vkDestroyPipeline(*device.handle, handle, nullptr);
}