#pragma once

#include <string>
#include <vector>

#include <vulkan/vulkan.h>

class Device;
class RenderPass;
class PipelineBuilder;

class Pipeline
{
    friend PipelineBuilder;

  private:
    std::weak_ptr<Device> device;

    VkDescriptorSetLayout descriptorSetLayout;
    VkPipelineLayout pipelineLayout;
    VkPipeline handle;

    VkExtent2D extent;

    Pipeline() = default;

  public:
    ~Pipeline();

    void recordBind(VkCommandBuffer &commandBuffer, uint32_t imageIndex);

  public:
    [[nodiscard]] const VkPipelineLayout &getPipelineLayout() const
    {
        return pipelineLayout;
    }
    [[nodiscard]] const VkDescriptorSetLayout &getDescriptorSetLayout() const
    {
        return descriptorSetLayout;
    }
};

class PipelineBuilder
{
  private:
    std::unique_ptr<Pipeline> product;

    std::weak_ptr<Device> device;

    // shaders
    std::vector<VkShaderModule> modules;
    std::vector<VkPipelineShaderStageCreateInfo> shaderStageCreateInfos;

    // dynamic states
    std::vector<VkDynamicState> dynamicStates;

    // draw mode
    VkPrimitiveTopology topology;
    bool bPrimitiveRestartEnable;

    VkExtent2D extent;

    // rasterizer
    VkBool32 depthClampEnable;
    VkBool32 rasterizerDiscardEnable;
    VkPolygonMode polygonMode;
    VkCullModeFlags cullMode;
    VkFrontFace frontFace;
    VkBool32 depthBiasEnable;
    float depthBiasConstantFactor;
    float depthBiasClamp;
    float depthBiasSlopeFactor;
    float lineWidth;

    // multisampling
    VkSampleCountFlagBits rasterizationSamples;
    VkBool32 sampleShadingEnable;
    float minSampleShading;
    const VkSampleMask *pSampleMask;
    VkBool32 alphaToCoverageEnable;
    VkBool32 alphaToOneEnable;

    // depth test
    VkBool32 depthTestEnable;
    VkBool32 depthWriteEnable;
    VkCompareOp depthCompareOp;
    VkBool32 depthBoundsTestEnable;
    VkBool32 stencilTestEnable;
    VkStencilOpState front;
    VkStencilOpState back;
    float minDepthBounds;
    float maxDepthBounds;

    // color blending
    VkBool32 blendEnable;
    VkBlendFactor srcColorBlendFactor;
    VkBlendFactor dstColorBlendFactor;
    VkBlendOp colorBlendOp;
    VkBlendFactor srcAlphaBlendFactor;
    VkBlendFactor dstAlphaBlendFactor;
    VkBlendOp alphaBlendOp;
    VkColorComponentFlags colorWriteMask;

    VkBool32 logicOpEnable;
    VkLogicOp logicOp;
    float blendConstants[4];

    // descriptor set layout
    std::vector<VkPushConstantRange> pushConstantRanges;

    RenderPass *renderPass;

    void restart();

  public:
    PipelineBuilder()
    {
        restart();
    }

    void setDevice(std::weak_ptr<Device> device)
    {
        this->device = device;
        product->device = device;
    }
    void addVertexShaderStage(const char *shaderName, const char *entryPoint = "main");
    void addFragmentShaderStage(const char *shaderName, const char *entryPoint = "main");
    void addDynamicState(VkDynamicState state);
    void setDrawTopology(VkPrimitiveTopology topology, bool bPrimitiveRestartEnable = false);
    void setExtent(VkExtent2D extent);
    void setDepthClampEnable(VkBool32 a)
    {
        depthClampEnable = a;
    }
    void setRasterizerDiscardEnable(VkBool32 a)
    {
        rasterizerDiscardEnable = a;
    }
    void setPolygonMode(VkPolygonMode a)
    {
        polygonMode = a;
    }
    void setCullMode(VkCullModeFlags a)
    {
        cullMode = a;
    }
    void setFrontFace(VkFrontFace a)
    {
        frontFace = a;
    }
    void setDepthBiasEnable(VkBool32 a)
    {
        depthBiasEnable = a;
    }
    void setDepthBiasConstantFactor(float a)
    {
        depthBiasConstantFactor = a;
    }
    void setDepthBiasClamp(float a)
    {
        depthBiasClamp = a;
    }
    void setDepthBiasSlopeFactor(float a)
    {
        depthBiasSlopeFactor = a;
    }
    void setLineWidth(float a)
    {
        lineWidth = a;
    }
    void setRasterizationSamples(VkSampleCountFlagBits a)
    {
        rasterizationSamples = a;
    }
    void setSampleShadingEnable(VkBool32 a)
    {
        sampleShadingEnable = a;
    }
    void setMinSampleShading(float a)
    {
        minSampleShading = a;
    }
    void setPSampleMask(const VkSampleMask *a)
    {
        pSampleMask = a;
    }
    void setAlphaToCoverageEnable(VkBool32 a)
    {
        alphaToCoverageEnable = a;
    }
    void setAlphaToOneEnable(VkBool32 a)
    {
        alphaToOneEnable = a;
    }
    void setDepthTestEnable(VkBool32 a)
    {
        depthTestEnable = a;
    }
    void setDepthWriteEnable(VkBool32 a)
    {
        depthWriteEnable = a;
    }
    void setDepthCompareOp(VkCompareOp a)
    {
        depthCompareOp = a;
    }
    void setDepthBoundsTestEnable(VkBool32 a)
    {
        depthBoundsTestEnable = a;
    }
    void setStencilTestEnable(VkBool32 a)
    {
        stencilTestEnable = a;
    }
    void setFront(VkStencilOpState a)
    {
        front = a;
    }
    void setBack(VkStencilOpState a)
    {
        back = a;
    }
    void setMinDepthBounds(float a)
    {
        minDepthBounds = a;
    }
    void setMaxDepthBounds(float a)
    {
        maxDepthBounds = a;
    }
    void setBlendEnable(VkBool32 a)
    {
        blendEnable = a;
    }
    void setSrcColorBlendFactor(VkBlendFactor a)
    {
        srcColorBlendFactor = a;
    }
    void setDstColorBlendFactor(VkBlendFactor a)
    {
        dstColorBlendFactor = a;
    }
    void setColorBlendOp(VkBlendOp a)
    {
        colorBlendOp = a;
    }
    void setSrcAlphaBlendFactor(VkBlendFactor a)
    {
        srcAlphaBlendFactor = a;
    }
    void setDstAlphaBlendFactor(VkBlendFactor a)
    {
        dstAlphaBlendFactor = a;
    }
    void setAlphaBlendOp(VkBlendOp a)
    {
        alphaBlendOp = a;
    }
    void setColorWriteMask(VkColorComponentFlags a)
    {
        colorWriteMask = a;
    }
    void setLogicOpEnable(VkBool32 a)
    {
        logicOpEnable = a;
    }
    void setLogicOp(VkLogicOp a)
    {
        logicOp = a;
    }
    void setBlendConstants(float a, float b, float c, float d)
    {
        blendConstants[0] = a;
        blendConstants[1] = b;
        blendConstants[2] = c;
        blendConstants[3] = d;
    }
    void setRenderPass(RenderPass *a)
    {
        renderPass = a;
    }

    std::unique_ptr<Pipeline> build();
};

class PipelineDirector
{
  public:
    void createColorDepthRasterizerBuilder(PipelineBuilder &build);
};
