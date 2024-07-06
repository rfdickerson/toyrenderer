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

VkShaderModule createShaderModule(Init& init, const std::vector<char>& code);

 std::vector<char> readFile(const std::string& filename);