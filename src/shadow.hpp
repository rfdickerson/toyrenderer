//
// Created by rfdic on 8/4/2024.
//

#ifndef TOYRENDERER_SHADOW_HPP
#define TOYRENDERER_SHADOW_HPP

namespace obsidian
{

// forward declarations
struct Init;
struct RenderData;

struct ShadowMap {
	VkImage image;
	VkImageView image_view;
	VmaAllocation allocation;
	VkSampler sampler;
	glm::mat4 light_space_matrix;
	glm::vec3 light_direction;
	float light_distance = 60.0f;
	float bias = 1.25f;
	float slope_bias = 1.75f;
	float radius = 5.0f;
	float near_plane = 1.0f;
	float far_plane = 100.0f;
};


void draw_shadow(Init &init, RenderData &data, VkCommandBuffer &command_buffer, uint32_t image_index);

void update_shadow(Init &init, RenderData &data);

// create the shadow map
void init_shadow_map(Init &init, RenderData &data);
void create_shadow_map(Init &init, RenderData& data, uint32_t width, uint32_t height);
void cleanup_shadow_map(Init &init, RenderData &data);

// create the shadow pipeline
void init_shadow_pipeline(Init &init, RenderData &data);


}

#endif        // TOYRENDERER_SHADOW_HPP
