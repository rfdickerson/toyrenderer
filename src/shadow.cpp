//
// Created by rfdic on 8/4/2024.
//

#include "shadow.hpp"

#include "common.hpp"
#include "utils.hpp"
#include "vk_mem_alloc.h"
#include "mesh.hpp"

namespace obsidian
{

constexpr uint32_t SHADOW_MAP_WIDTH  = 2048;
constexpr uint32_t SHADOW_MAP_HEIGHT = 2048;
constexpr VkFormat SHADOW_MAP_FORMAT = VK_FORMAT_D16_UNORM;

struct AllocatedImage
{
	VkImage       image;
	VmaAllocation allocation;
	VkImageView   image_view;
	VkSampler     sampler;
};

AllocatedImage create_shadow_map_image(Init &init, uint32_t width, uint32_t height)
{
	VkImageCreateInfo image_info = {};
	image_info.sType             = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	image_info.imageType         = VK_IMAGE_TYPE_2D;
	image_info.extent.width      = width;
	image_info.extent.height     = height;
	image_info.extent.depth      = 1;
	image_info.mipLevels         = 1;
	image_info.arrayLayers       = 1;
	image_info.format            = SHADOW_MAP_FORMAT;
	image_info.tiling            = VK_IMAGE_TILING_OPTIMAL;
	image_info.initialLayout     = VK_IMAGE_LAYOUT_UNDEFINED;
	image_info.usage             = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
	image_info.samples           = VK_SAMPLE_COUNT_1_BIT;

	VmaAllocationCreateInfo allocation_create_info = {};
	allocation_create_info.usage                   = VMA_MEMORY_USAGE_GPU_ONLY;

	AllocatedImage allocated_image;
	vmaCreateImage(init.allocator, &image_info, &allocation_create_info, &allocated_image.image, &allocated_image.allocation, nullptr);

	return allocated_image;
}

void create_image_view(Init &init, AllocatedImage &allocated_image)
{
	VkImageViewCreateInfo view_info           = {};
	view_info.sType                           = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	view_info.image                           = allocated_image.image;
	view_info.viewType                        = VK_IMAGE_VIEW_TYPE_2D;
	view_info.format                          = SHADOW_MAP_FORMAT;
	view_info.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_DEPTH_BIT;
	view_info.subresourceRange.baseMipLevel   = 0;
	view_info.subresourceRange.levelCount     = 1;
	view_info.subresourceRange.baseArrayLayer = 0;
	view_info.subresourceRange.layerCount     = 1;

	vkCreateImageView(init.device, &view_info, nullptr, &allocated_image.image_view);
}

void create_sampler(Init &init, AllocatedImage &allocated_image)
{
	VkSamplerCreateInfo sampler_info     = {};
	sampler_info.sType                   = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	sampler_info.magFilter               = VK_FILTER_LINEAR;
	sampler_info.minFilter               = VK_FILTER_LINEAR;
	sampler_info.addressModeU            = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
	sampler_info.addressModeV            = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
	sampler_info.addressModeW            = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
	sampler_info.anisotropyEnable        = VK_FALSE;
	sampler_info.maxAnisotropy           = 1.0f;
	sampler_info.borderColor             = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
	sampler_info.unnormalizedCoordinates = VK_FALSE;
	sampler_info.compareEnable           = VK_TRUE;
	sampler_info.compareOp               = VK_COMPARE_OP_LESS_OR_EQUAL;
	sampler_info.mipmapMode              = VK_SAMPLER_MIPMAP_MODE_NEAREST;
	sampler_info.mipLodBias              = 0.0f;
	sampler_info.minLod                  = 0.0f;
	sampler_info.maxLod                  = 0.0f;

	vkCreateSampler(init.device, &sampler_info, nullptr, &allocated_image.sampler);
}

void cleanup_shadow_map(Init &init, RenderData &data)
{
	vkDestroySampler(init.device, data.shadow_map.sampler, nullptr);
	vkDestroyImageView(init.device, data.shadow_map.image_view, nullptr);
	vmaDestroyImage(init.allocator, data.shadow_map.image, data.shadow_map.allocation);
}

void create_shadow_map(Init &init, RenderData& data, uint32_t width, uint32_t height)
{
	AllocatedImage allocated_image = create_shadow_map_image(init, width, height);
	// create image view
	create_image_view(init, allocated_image);
	// create sampler
	create_sampler(init, allocated_image);

	data.shadow_map.image = allocated_image.image;
	data.shadow_map.allocation = allocated_image.allocation;
	data.shadow_map.image_view = allocated_image.image_view;
	data.shadow_map.sampler = allocated_image.sampler;


}

void draw_shadow(Init &init, RenderData &data, VkCommandBuffer &command_buffer, uint32_t image_index)
{
	// create dynamic render pass
	VkRenderingAttachmentInfo attachment_info = {};
	attachment_info.sType                     = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
	attachment_info.pNext                     = nullptr;
	attachment_info.imageView                 = data.shadow_map.image_view;
	attachment_info.imageLayout               = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	attachment_info.resolveImageView          = nullptr;
	attachment_info.resolveImageLayout        = VK_IMAGE_LAYOUT_UNDEFINED;
	attachment_info.loadOp                    = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachment_info.storeOp                   = VK_ATTACHMENT_STORE_OP_STORE;
	attachment_info.clearValue.depthStencil   = {1.0f, 0};

	VkRenderingInfo render_info      = {};
	render_info.sType                = VK_STRUCTURE_TYPE_RENDERING_INFO;
	render_info.pNext                = nullptr;
	render_info.flags                = 0;
	render_info.pColorAttachments    = nullptr;
	render_info.colorAttachmentCount = 0;
	render_info.pDepthAttachment     = &attachment_info;
	render_info.renderArea.offset    = {0, 0};
	render_info.renderArea.extent    = {SHADOW_MAP_WIDTH, SHADOW_MAP_HEIGHT};
	render_info.layerCount		   	 = 1;
	render_info.viewMask			 = 0;

	init.disp.cmdBeginRendering(command_buffer, &render_info);

	// Set viewport
	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = static_cast<float>(SHADOW_MAP_WIDTH);
	viewport.height = static_cast<float>(SHADOW_MAP_HEIGHT);
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	// Set scissor
	VkRect2D scissor{};
	scissor.offset = {0, 0};
	scissor.extent = {SHADOW_MAP_WIDTH, SHADOW_MAP_HEIGHT};

	// set dynamic states
	init.disp.cmdSetDepthBias(command_buffer, data.shadow_map.bias, 0.0f, data.shadow_map.slope_bias);
	init.disp.cmdSetViewport(command_buffer, 0, 1, &viewport);
	init.disp.cmdSetScissor(command_buffer, 0, 1, &scissor);

	// draw scene
	// bind pipeline
	init.disp.cmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, data.shadow_pipeline);

	// bind descriptor set
	init.disp.cmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, data.shadow_pipeline_layout, 0, 1, &data.descriptor_sets[image_index], 0, nullptr);

	data.bunny_mesh->draw(init, command_buffer);
	//data.mesh->draw(init, command_buffer);
	//data.plane_mesh->draw(init, command_buffer);

	init.disp.cmdEndRendering(command_buffer);
}

glm::mat4 calculate_light_space_matrix(const glm::vec3 &light_direction,
                                       const glm::vec3 &scene_center,
                                       float            scene_radius,
                                       float light_distance = 25.0f,
                                       float near_plane = 1.0f,
                                       float far_plane = 7.5f
                                       )
{
	glm::vec3 light_position = scene_center - light_direction * light_distance;

	glm::mat4 light_view = glm::lookAt(
	    light_position,
	    scene_center,
	    glm::vec3(0.0f, 1.0f, 0.0f)
	);

	glm::mat4 light_projection = glm::ortho(
	    -scene_radius, scene_radius,
	    -scene_radius, scene_radius,
	    near_plane, far_plane
	);

	return light_projection * light_view;
}

void init_shadow_map(Init &init, RenderData &data) {
	create_shadow_map(init, data, SHADOW_MAP_WIDTH, SHADOW_MAP_HEIGHT);

	const glm::vec3 light_direction = glm::normalize(glm::vec3(-1.0f, -1.0f, -2.2f));
	const glm::vec3 center = glm::vec3(0.0f);

	data.shadow_map.light_direction = light_direction;

	update_shadow(init, data);

}

void update_shadow(Init &init, RenderData &data) {
	const glm::vec3 center = glm::vec3(0.0f);

	data.shadow_map.light_space_matrix = calculate_light_space_matrix(
	    data.shadow_map.light_direction,
	    center,
	    data.shadow_map.radius,
	    data.shadow_map.light_distance,
	    data.shadow_map.near_plane,
	    data.shadow_map.far_plane
	    );
}

VkPipelineLayout create_shadow_pipeline_layout(Init& init, RenderData& data) {
	VkPipelineLayoutCreateInfo pipeline_layout_info = {};
	pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipeline_layout_info.setLayoutCount = 1;
	pipeline_layout_info.pSetLayouts = &data.descriptor_set_layout;
	pipeline_layout_info.pushConstantRangeCount = 0;
	pipeline_layout_info.pPushConstantRanges = nullptr;

	VkPipelineLayout pipeline_layout;
	vkCreatePipelineLayout(init.device, &pipeline_layout_info, nullptr, &pipeline_layout);

	return pipeline_layout;
}

VkPipeline create_shadow_pipeline(Init &init, VkPipelineLayout pipeline_layout) {

	auto vert_code = read_file("shaders/shadow.vert.spv");

	VkShaderModule vert_shader_module = create_shader_module(init, vert_code);

	VkPipelineShaderStageCreateInfo vert_shader_stage_info = {};
	vert_shader_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vert_shader_stage_info.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vert_shader_stage_info.module = vert_shader_module;
	vert_shader_stage_info.pName = "main";

	VkPipelineShaderStageCreateInfo shader_stages[] = {vert_shader_stage_info};

	VkVertexInputBindingDescription bindingDescription;
	bindingDescription.binding = 0;
	bindingDescription.stride = sizeof(Vertex);
	bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	std::array<VkVertexInputAttributeDescription, 1> attributeDescriptions{};

	attributeDescriptions[0].binding = 0;
	attributeDescriptions[0].location = 0;
	attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
	attributeDescriptions[0].offset = offsetof(Vertex, pos);

	VkPipelineVertexInputStateCreateInfo vertex_input_info = {};
	vertex_input_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertex_input_info.vertexBindingDescriptionCount = 1;
	vertex_input_info.pVertexBindingDescriptions = &bindingDescription;
	vertex_input_info.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
	vertex_input_info.pVertexAttributeDescriptions = attributeDescriptions.data();

	VkPipelineInputAssemblyStateCreateInfo input_assembly = {};
	input_assembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	input_assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	input_assembly.primitiveRestartEnable = VK_FALSE;

	VkViewport viewport = {};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = SHADOW_MAP_WIDTH;
	viewport.height = SHADOW_MAP_HEIGHT;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	VkRect2D scissor = {};
	scissor.offset = {0, 0};
	scissor.extent = {SHADOW_MAP_WIDTH, SHADOW_MAP_HEIGHT};

	VkPipelineViewportStateCreateInfo viewport_state = {};
	viewport_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewport_state.viewportCount = 1;
	viewport_state.pViewports = &viewport;
	viewport_state.scissorCount = 1;
	viewport_state.pScissors = &scissor;

	VkPipelineRasterizationStateCreateInfo rasterizer = {};
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable = VK_FALSE;
	rasterizer.rasterizerDiscardEnable = VK_FALSE;
	rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizer.lineWidth = 1.0f;
	rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;

	rasterizer.depthBiasEnable = VK_TRUE;
	rasterizer.depthBiasConstantFactor = 1.0f;
	rasterizer.depthBiasClamp = 0.0f;
	rasterizer.depthBiasSlopeFactor = 1.0f;

	VkPipelineMultisampleStateCreateInfo multisampling = {};
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

	VkPipelineDepthStencilStateCreateInfo depth_stencil = {};
	depth_stencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depth_stencil.depthTestEnable = VK_TRUE;
	depth_stencil.depthWriteEnable = VK_TRUE;
	depth_stencil.depthCompareOp = VK_COMPARE_OP_LESS;
	depth_stencil.depthBoundsTestEnable = VK_FALSE;
	depth_stencil.stencilTestEnable = VK_FALSE;

	VkPipelineColorBlendStateCreateInfo color_blending{};
	color_blending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	color_blending.logicOpEnable = VK_FALSE;
	color_blending.attachmentCount = 0; // we're only writing to the depth attachment
	color_blending.pAttachments = nullptr;

	std::vector<VkDynamicState> dynamic_states = {
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR,
	    VK_DYNAMIC_STATE_DEPTH_BIAS
	};

	VkPipelineDynamicStateCreateInfo dynamic_state = {};
	dynamic_state.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamic_state.dynamicStateCount = static_cast<uint32_t>(dynamic_states.size());
	dynamic_state.pDynamicStates = dynamic_states.data();

	VkGraphicsPipelineCreateInfo pipeline_info = {};
	pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipeline_info.stageCount = 1;
	pipeline_info.pStages = shader_stages;
	pipeline_info.pVertexInputState = &vertex_input_info;
	pipeline_info.pInputAssemblyState = &input_assembly;
	pipeline_info.pViewportState = &viewport_state;
	pipeline_info.pRasterizationState = &rasterizer;
	pipeline_info.pMultisampleState = &multisampling;
	pipeline_info.pDepthStencilState = &depth_stencil;
	pipeline_info.pColorBlendState = &color_blending;
	pipeline_info.pDynamicState = &dynamic_state;
	pipeline_info.layout = pipeline_layout;
	pipeline_info.renderPass = nullptr;
	pipeline_info.subpass = 0;

	VkPipelineRenderingCreateInfo pipeline_rendering_create_info = {};
	pipeline_rendering_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
	pipeline_rendering_create_info.depthAttachmentFormat = SHADOW_MAP_FORMAT;
	pipeline_rendering_create_info.colorAttachmentCount = 0;
	pipeline_rendering_create_info.pColorAttachmentFormats = nullptr;

	pipeline_info.pNext = &pipeline_rendering_create_info;

	VkPipeline pipeline;
	if (init.disp.createGraphicsPipelines(VK_NULL_HANDLE, 1, &pipeline_info, nullptr, &pipeline) != VK_SUCCESS) {
		throw std::runtime_error("failed to create shadow graphics pipeline!");
	}

	init.disp.destroyShaderModule(vert_shader_module, nullptr);

	return pipeline;
}



void init_shadow_pipeline(Init &init, RenderData &data) {
	data.shadow_pipeline_layout = create_shadow_pipeline_layout(init, data);
	data.shadow_pipeline = create_shadow_pipeline(init, data.shadow_pipeline_layout);
}

}		// namespace obsidian