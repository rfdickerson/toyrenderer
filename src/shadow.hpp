//
// Created by rfdic on 8/4/2024.
//

#ifndef TOYRENDERER_SHADOW_HPP
#define TOYRENDERER_SHADOW_HPP

namespace obsidian
{
struct Init;

struct ShadowMap {
	VkImage image;
	VkImageView image_view;
	VmaAllocation allocation;
	VkSampler sampler;

};

ShadowMap create_shadow_map(Init &init, uint32_t width, uint32_t height);

void cleanup_shadow_map(Init &init, ShadowMap &shadow_map);

}

#endif        // TOYRENDERER_SHADOW_HPP
