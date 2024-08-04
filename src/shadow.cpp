//
// Created by rfdic on 8/4/2024.
//

#include "shadow.hpp"

#include "common.hpp"
#include "vk_mem_alloc.h"

namespace obsidian
{

constexpr uint32_t SHADOW_MAP_WIDTH = 2048;
constexpr uint32_t SHADOW_MAP_HEIGHT = 2048;
constexpr VkFormat SHADOW_MAP_FORMAT = VK_FORMAT_D32_SFLOAT;

struct AllocatedImage {
	VkImage image;
	VmaAllocation allocation;
	VkImageView image_view;
	VkSampler sampler;
};

AllocatedImage create_shadow_map_image(Init &init, uint32_t width, uint32_t height) {
	VkImageCreateInfo image_info = {};
	image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	image_info.imageType = VK_IMAGE_TYPE_2D;
	image_info.extent.width = width;
	image_info.extent.height = height;
	image_info.extent.depth = 1;
	image_info.mipLevels = 1;
	image_info.arrayLayers = 1;
	image_info.format = SHADOW_MAP_FORMAT;
	image_info.tiling = VK_IMAGE_TILING_OPTIMAL;
	image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	image_info.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;

	VmaAllocationCreateInfo allocation_create_info = {};
	allocation_create_info.usage = VMA_MEMORY_USAGE_GPU_ONLY;

	AllocatedImage allocated_image;
	vmaCreateImage(init.allocator, &image_info, &allocation_create_info, &allocated_image.image, &allocated_image.allocation, nullptr);

	return allocated_image;
}

void create_image_view(Init &init, AllocatedImage &allocated_image) {
	VkImageViewCreateInfo view_info = {};
	view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	view_info.image = allocated_image.image;
	view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
	view_info.format = SHADOW_MAP_FORMAT;
	view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
	view_info.subresourceRange.baseMipLevel = 0;
	view_info.subresourceRange.levelCount = 1;
	view_info.subresourceRange.baseArrayLayer = 0;
	view_info.subresourceRange.layerCount = 1;

	vkCreateImageView(init.device, &view_info, nullptr, &allocated_image.image_view);
}

void create_sampler(Init &init, AllocatedImage &allocated_image) {
	VkSamplerCreateInfo sampler_info = {};
	sampler_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	sampler_info.magFilter = VK_FILTER_LINEAR;
	sampler_info.minFilter = VK_FILTER_LINEAR;
	sampler_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	sampler_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	sampler_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	sampler_info.anisotropyEnable = VK_FALSE;
	sampler_info.maxAnisotropy = 1.0f;
	sampler_info.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
	sampler_info.unnormalizedCoordinates = VK_FALSE;
	sampler_info.compareEnable = VK_FALSE;
	sampler_info.compareOp = VK_COMPARE_OP_ALWAYS;
	sampler_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	sampler_info.mipLodBias = 0.0f;
	sampler_info.minLod = 0.0f;
	sampler_info.maxLod = 1.0f;

	vkCreateSampler(init.device, &sampler_info, nullptr, &allocated_image.sampler);
}

void cleanup_shadow_map_image(Init &init, AllocatedImage &allocated_image) {
	vmaDestroyImage(init.allocator, allocated_image.image, allocated_image.allocation);
}



ShadowMap create_shadow_map(Init &init, uint32_t width, uint32_t height) {

	ShadowMap shadow_map {};
	// create image

	AllocatedImage allocated_image = create_shadow_map_image(init, width, height);
	// create image view
	create_image_view(init, allocated_image);
	// create sampler
	create_sampler(init, allocated_image);

	shadow_map.image = allocated_image.image;

	return shadow_map;

}

}