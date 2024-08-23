//
// Created by rfdic on 8/20/2024.
//
#include "buffer.hpp"

#include <vk_mem_alloc.h>

#include "common.hpp"
#include "utils.hpp"


namespace obsidian
{

BufferAllocation create_buffer(
	const Init              &init,
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

	BufferAllocation alloc {
		.buffer = VK_NULL_HANDLE,
		.allocation = nullptr,
		.size = 0,
		.type = BufferType::Undefined,
	};

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

BufferAllocation create_staging_buffer(const Init &init, uint32_t size)
{
	BufferAllocation staging_buffer {};

	VkBufferCreateInfo buffer_info = {
	    .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
	    .size = size,
	    .usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
	    .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
	};

	VmaAllocationCreateInfo alloc_info = {
	    .flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT |
	             VMA_ALLOCATION_CREATE_MAPPED_BIT,
	    .usage = VMA_MEMORY_USAGE_AUTO,
	};

	if (vmaCreateBuffer(init.allocator, &buffer_info, &alloc_info, &staging_buffer.buffer, &staging_buffer.allocation, nullptr) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create staging buffer!");
	}

	staging_buffer.size = size;
	staging_buffer.type = BufferType::StagingBuffer;

	return staging_buffer;
}

void cleanup_buffer(const Init &init, BufferAllocation &bufferAllocation)
{
	vmaDestroyBuffer(init.allocator, bufferAllocation.buffer, bufferAllocation.allocation);
	bufferAllocation.buffer = VK_NULL_HANDLE;
	bufferAllocation.allocation = nullptr;
}

// copy data to buffer at offset
void copy_buffer_data(const Init& init, BufferAllocation& buffer, VkDeviceSize offset, VkDeviceSize size, const void* data) {
	void *mapped_data;
	VkResult result = vmaMapMemory(init.allocator, buffer.allocation, &mapped_data);
	if (result != VK_SUCCESS) {
		throw std::runtime_error("failed to map memory!");
	}

	if ( size <= 0 || offset + size > buffer.size) {
		vmaUnmapMemory(init.allocator, buffer.allocation);
		throw std::runtime_error("invalid offset or size!");
	}

	memcpy(static_cast<char *>(mapped_data) + offset, data, size);
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