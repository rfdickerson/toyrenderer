//
// Created by rfdic on 8/20/2024.
//
#include "buffer.hpp"

#include "common.hpp"
#include "utils.hpp"

namespace obsidian
{

BufferAllocation create_buffer(const Init              &init,
				   const VkDeviceSize       size,
				   const VkBufferUsageFlags usage,
				   const VmaMemoryUsage     memoryUsage)
{

	const VkBufferCreateInfo bufferInfo = {
		.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
		.size = size,
		.usage = usage,
		.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
	};

	const VmaAllocationCreateInfo allocInfo = {
		.usage = memoryUsage,
	};

	BufferAllocation alloc;

	if (vmaCreateBuffer(init.allocator, &bufferInfo, &allocInfo,
						&alloc.buffer,
						&alloc.allocation,
						nullptr) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create buffer!");
	}

	alloc.size = size;

	return alloc;

}

void cleanup_buffer(const Init &init, BufferAllocation &bufferAllocation)
{
	vmaDestroyBuffer(init.allocator, bufferAllocation.buffer, bufferAllocation.allocation);
	bufferAllocation.buffer = VK_NULL_HANDLE;
	bufferAllocation.allocation = nullptr;
}

void copy_buffer_data(Init& init, BufferAllocation buffer, VkDeviceSize size, const void* data) {
	void *mapped_data;
	vmaMapMemory(init.allocator, buffer.allocation, &mapped_data);
	memcpy(mapped_data, data, size);
	vmaUnmapMemory(init.allocator, buffer.allocation);
}

VkResult copy_buffer(const Init& init, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
{
	VkCommandBuffer commandBuffer = begin_single_time_commands(init);

	VkBufferCopy copyRegion = {};
	copyRegion.size         = size;
	init.disp.cmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

	end_single_time_commands(init, commandBuffer);

	return VK_SUCCESS;
}

}