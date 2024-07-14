 #pragma once

#include "common.hpp"

VkCommandBuffer begin_single_time_commands(Init& init);
void end_single_time_commands(Init& init, VkCommandBuffer commandBuffer);

void create_buffer(Init& init,
                   VkDeviceSize size,
                   VkBufferUsageFlags usage,
                   VmaMemoryUsage memoryUsage,
                   BufferAllocation& bufferAllocation);

void cleanup_buffer(Init& init, BufferAllocation& bufferAllocation);

std::vector<char> read_file(const std::string& filename);
VkShaderModule create_shader_module(Init& init, const std::vector<char>& code);
