#pragma once

#include <vulkan/vulkan.h>
#include <vector>

namespace obsidian
{
struct Init;
struct BufferAllocation;

// Single use command buffer submission
VkCommandBuffer begin_single_time_commands(const Init& init);

void end_single_time_commands(const Init& init, const VkCommandBuffer commandBuffer);

std::vector<char> read_file(const std::string &filename);

VkShaderModule    create_shader_module(const Init &init, const std::vector<char> &code);





} // namespace obsidian