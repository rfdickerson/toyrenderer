//
// Created by rfdic on 8/19/2024.
//

#ifndef BUFFER_ALLOCATION_HPP
#define BUFFER_ALLOCATION_HPP

#include <vk_mem_alloc.h>

namespace obsidian
{

struct Init;

enum class BufferType
{
	VertexBuffer,
	IndexBuffer,
	UniformBuffer,
};

struct BufferAllocation
{
	VkBuffer buffer;
	VmaAllocation allocation;
	VkDeviceSize size;
	BufferType type;
};

// utility functions
BufferAllocation create_buffer(
	const Init& init,
	VkDeviceSize       size,
	VkBufferUsageFlags usage,
	VmaMemoryUsage     memoryUsage);

void cleanup_buffer(const Init& init, BufferAllocation &bufferAllocation);

// copy data from CPU to Vulkan buffer
void copy_buffer_data(Init& init, BufferAllocation buffer, VkDeviceSize size, const void* data);

// copy one Vulkan buffer to another
VkResult copy_buffer(const Init& init, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

}

#endif //BUFFER_ALLOCATION_HPP
