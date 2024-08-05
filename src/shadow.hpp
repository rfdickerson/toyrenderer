//
// Created by rfdic on 8/4/2024.
//

#ifndef TOYRENDERER_SHADOW_HPP
#define TOYRENDERER_SHADOW_HPP

namespace obsidian
{
struct Init;
struct RenderData;

struct ShadowMap {
	VkImage image;
	VkImageView image_view;
	VmaAllocation allocation;
	VkSampler sampler;
};

struct ShadowUniformBufferObject {
	glm::mat4 light_space_matrix;
	glm::vec3 light_direction;
	float padding;
};

void draw_shadow(Init &init, RenderData &data, VkCommandBuffer &command_buffer);

// create the shadow map
void init_shadow_map(Init &init, RenderData &data);
void create_shadow_map(Init &init, RenderData& data, uint32_t width, uint32_t height);
void cleanup_shadow_map(Init &init, RenderData &data);
void update_shadow_uniform_buffer(Init &init, RenderData &data);
void update_descriptor_set(Init &init, RenderData &data);

// create the shadow pipeline
void init_shadow_pipeline(Init &init, RenderData &data);


}

#endif        // TOYRENDERER_SHADOW_HPP
