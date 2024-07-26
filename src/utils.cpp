#include "utils.hpp"

#include <vulkan/vulkan.h>

#include "common.hpp"

namespace obsidian
{

// Add these helper functions for command buffer operations
VkCommandBuffer begin_single_time_commands(Init &init)
{
	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool        = init.command_pool;
	allocInfo.commandBufferCount = 1;

	VkCommandBuffer commandBuffer;
	init.disp.allocateCommandBuffers(&allocInfo, &commandBuffer);

	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	init.disp.beginCommandBuffer(commandBuffer, &beginInfo);

	return commandBuffer;
}

void end_single_time_commands(Init &init, VkCommandBuffer commandBuffer)
{
	init.disp.endCommandBuffer(commandBuffer);

	VkSubmitInfo submitInfo{};
	submitInfo.sType              = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers    = &commandBuffer;

	init.disp.queueSubmit(init.graphics_queue, 1, &submitInfo, VK_NULL_HANDLE);
	init.disp.queueWaitIdle(init.graphics_queue);

	init.disp.freeCommandBuffers(init.command_pool, 1, &commandBuffer);
}

void create_buffer(Init              &init,
                   VkDeviceSize       size,
                   VkBufferUsageFlags usage,
                   VmaMemoryUsage     memoryUsage,
                   BufferAllocation  &bufferAllocation)
{
	VkBufferCreateInfo bufferInfo = {};
	bufferInfo.sType              = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size               = size;
	bufferInfo.usage              = usage;
	bufferInfo.sharingMode        = VK_SHARING_MODE_EXCLUSIVE;

	VmaAllocationCreateInfo allocInfo = {};
	allocInfo.usage                   = memoryUsage;

	if (vmaCreateBuffer(init.allocator, &bufferInfo, &allocInfo,
	                    &bufferAllocation.buffer,
	                    &bufferAllocation.allocation,
	                    nullptr) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create buffer!");
	}

	bufferAllocation.size = size;
}

void cleanup_buffer(Init &init, BufferAllocation &bufferAllocation)
{
	vmaDestroyBuffer(init.allocator, bufferAllocation.buffer, bufferAllocation.allocation);
	bufferAllocation.buffer = VK_NULL_HANDLE;
}

std::vector<char> read_file(const std::string &filename)
{
	std::ifstream file(filename, std::ios::ate | std::ios::binary);

	if (!file.is_open())
	{
		throw std::runtime_error("failed to open file!");
	}

	size_t            file_size = (size_t) file.tellg();
	std::vector<char> buffer(file_size);

	file.seekg(0);
	file.read(buffer.data(), static_cast<std::streamsize>(file_size));

	file.close();

	return buffer;
}

VkShaderModule create_shader_module(Init &init, const std::vector<char> &code)
{
	VkShaderModuleCreateInfo create_info = {};
	create_info.sType                    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	create_info.codeSize                 = code.size();
	create_info.pCode                    = reinterpret_cast<const uint32_t *>(code.data());

	VkShaderModule shaderModule;
	if (init.disp.createShaderModule(&create_info, nullptr, &shaderModule) != VK_SUCCESS)
	{
		return VK_NULL_HANDLE;        // failed to create shader module
	}

	return shaderModule;
}

VkResult copy_buffer(Init &init, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
{
	VkCommandBuffer commandBuffer = begin_single_time_commands(init);

	VkBufferCopy copyRegion = {};
	copyRegion.size         = size;
	init.disp.cmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

	end_single_time_commands(init, commandBuffer);

	return VK_SUCCESS;
}

void transition_image_to_color_attachment(Init &init, const VkCommandBuffer& command_buffer, const VkImage& image) {
	VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    barrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

	init.disp.cmdPipelineBarrier(command_buffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);
}

void transition_image_to_depth_attachment(Init &init, const VkCommandBuffer& command_buffer, const VkImage& image)
{
	VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    barrier.newLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;
	barrier.srcAccessMask = 0;
	barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;

    init.disp.cmdPipelineBarrier(command_buffer,
        VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);
}

void transition_image_to_present(Init &init, const VkCommandBuffer& command_buffer, const VkImage& image)
{
	VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;
    barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
	barrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;

	init.disp.cmdPipelineBarrier(command_buffer, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);
}

}