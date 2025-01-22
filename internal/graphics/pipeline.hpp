#pragma once

#include <string>
#include <vector>

#include <vulkan/vulkan.h>

class Device;
class RenderPass;

class Pipeline
{
  private:
    const Device &device;

  public:
    VkDescriptorSetLayout descriptorSetLayout;
    VkPipelineLayout pipelineLayout;
    VkPipeline handle;

    VkShaderModule create_shader_module(VkDevice device, const std::vector<char> &code);
    void destroy_shader_module(VkDevice device, VkShaderModule module);
    bool read_binary_file(const std::string &filename, std::vector<char> &out);

  public:
    Pipeline(const Device &device, const char *shaderName, const RenderPass &renderPass, const VkExtent2D &extent);
    ~Pipeline();
};