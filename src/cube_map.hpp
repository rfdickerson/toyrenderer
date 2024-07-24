#pragma once

namespace obsidian
{
struct Init;
struct RenderData;

class CubeMap
{
  public:
	CubeMap(Init &init, RenderData &renderData);
	~CubeMap();

	Init       &init;
	RenderData &renderData;

	VkPipeline       pipeline;
	VkPipelineLayout pipeline_layout;

	VkRenderPass render_pass;

	VkResult createRenderPass();
	VkResult createPipeline();
	VkResult createPipelineLayout();

	VkResult render(Init& init, RenderData& render_data, VkCommandBuffer command_buffer, uint32_t image_index);
};

} // namespace obsidian
