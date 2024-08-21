#pragma once

namespace obsidian
{
struct Init;
struct RenderData;

struct CubeMap
{
	VkPipeline       pipeline;
	VkPipelineLayout pipeline_layout;
	VkDescriptorSetLayout descriptor_set_layout;
	VkDescriptorSet       descriptor_set;
};

CubeMap create_cubemap(const Init& init, VkDescriptorSetLayout descriptor_set_layout);

void cleanup_cubemap(const Init& init, CubeMap& cubemap);

void render_cubemap(const Init& init,
	const RenderData& render_data,
	const CubeMap& cube_map,
	VkCommandBuffer command_buffer,
	uint32_t image_index);

VkPipelineLayout create_cubemap_pipeline_layout(const Init& init, VkDescriptorSetLayout);
VkPipeline create_cubemap_pipeline(const Init& init, VkPipelineLayout pipeline_layout);

} // namespace obsidian
