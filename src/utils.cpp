#include <vulkan/vulkan.h>

#include "common.hpp"

// Add these helper functions for command buffer operations
VkCommandBuffer begin_single_time_commands(Init& init) {
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = init.command_pool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    init.disp.allocateCommandBuffers(&allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    init.disp.beginCommandBuffer(commandBuffer, &beginInfo);

    return commandBuffer;
}

void end_single_time_commands(Init& init, VkCommandBuffer commandBuffer) {
    init.disp.endCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    init.disp.queueSubmit(init.graphics_queue, 1, &submitInfo, VK_NULL_HANDLE);
    init.disp.queueWaitIdle(init.graphics_queue);

    init.disp.freeCommandBuffers(init.command_pool, 1, &commandBuffer);
}

void create_buffer(Init& init,
                   VkDeviceSize size,
                   VkBufferUsageFlags usage,
                   VmaMemoryUsage memoryUsage,
                   BufferAllocation& bufferAllocation) {

    VkBufferCreateInfo bufferInfo = {};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VmaAllocationCreateInfo allocInfo = {};
    allocInfo.usage = memoryUsage;

    if (vmaCreateBuffer(init.allocator, &bufferInfo, &allocInfo,
                        &bufferAllocation.buffer,
                        &bufferAllocation.allocation,
                        nullptr) != VK_SUCCESS) {
        throw std::runtime_error("failed to create buffer!");
    }

    bufferAllocation.size = size;
}

void cleanup_buffer(Init& init, BufferAllocation& bufferAllocation) {
    vmaDestroyBuffer(init.allocator, bufferAllocation.buffer, bufferAllocation.allocation);
    bufferAllocation.buffer = VK_NULL_HANDLE;
}