#pragma once

#include <vulkan/vulkan.h>
#include <vector>

namespace obsidian
{
struct Init;
struct BufferAllocation;

VkCommandBuffer begin_single_time_commands(Init &init);
void            end_single_time_commands(Init &init, VkCommandBuffer commandBuffer);

void create_buffer(Init              &init,
                   VkDeviceSize       size,
                   VkBufferUsageFlags usage,
                   VmaMemoryUsage     memoryUsage,
                   BufferAllocation  &bufferAllocation);

void cleanup_buffer(Init &init, BufferAllocation &bufferAllocation);

std::vector<char> read_file(const std::string &filename);
VkShaderModule    create_shader_module(Init &init, const std::vector<char> &code);

VkResult copy_buffer(Init &init, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

void transition_image_to_color_attachment(Init &init, const VkCommandBuffer& command_buffer, const VkImage& image);

void transition_image_to_depth_attachment(Init &init, const VkCommandBuffer& command_buffer, const VkImage& image);

void transition_image_to_present(Init &init, const VkCommandBuffer& command_buffer, const VkImage& image);


} // namespace obsidian