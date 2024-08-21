//
// Created by rfdic on 7/13/2024.
//

#include "cube_map.hpp"

#include "common.hpp"
#include "utils.hpp"
 
namespace obsidian
{

CubeMap create_cubemap(const Init &init, VkDescriptorSetLayout setLayout)
{
	VkPipelineLayout pipeline_layout = create_cubemap_pipeline_layout(init, setLayout);
	VkPipeline pipeline = create_cubemap_pipeline(init, pipeline_layout);

	return CubeMap {
		.pipeline = pipeline,
		.pipeline_layout = pipeline_layout
	};
}

void cleanup_cubemap(const Init& init, CubeMap &cubemap)
{
	vkDestroyPipeline(init.device, cubemap.pipeline, nullptr);
	vkDestroyPipelineLayout(init.device, cubemap.pipeline_layout, nullptr);

	cubemap.pipeline = VK_NULL_HANDLE;
	cubemap.pipeline_layout = VK_NULL_HANDLE;
}

VkPipelineLayout create_cubemap_pipeline_layout(const Init &init, VkDescriptorSetLayout set_layout)
{

	VkPipelineLayoutCreateInfo pipeline_layout_info = {};
	pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipeline_layout_info.setLayoutCount = 1;
	pipeline_layout_info.pSetLayouts = &set_layout;
	pipeline_layout_info.pushConstantRangeCount = 0;
	pipeline_layout_info.pPushConstantRanges = nullptr;

	VkPipelineLayout pipeline_layout = VK_NULL_HANDLE;

	if (vkCreatePipelineLayout(init.device, &pipeline_layout_info, nullptr, &pipeline_layout) != VK_SUCCESS) {
		throw std::runtime_error("failed to create pipeline layout!");
	}

	return pipeline_layout;
}



VkPipeline create_cubemap_pipeline(const Init &init, VkPipelineLayout pipeline_layout) {

	VkShaderModule vert_shader_module = create_shader_module(init, read_file("shaders/cubemap.vert.spv"));
	VkShaderModule frag_shader_module = create_shader_module(init, read_file("shaders/cubemap.frag.spv"));

	VkPipelineShaderStageCreateInfo vert_shader_stage_info = {};
	vert_shader_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vert_shader_stage_info.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vert_shader_stage_info.module = vert_shader_module;
	vert_shader_stage_info.pName = "main";

	VkPipelineShaderStageCreateInfo frag_shader_stage_info = {};
	frag_shader_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	frag_shader_stage_info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	frag_shader_stage_info.module = frag_shader_module;
	frag_shader_stage_info.pName = "main";

	VkPipelineShaderStageCreateInfo shader_stages[] = {vert_shader_stage_info, frag_shader_stage_info};

	VkPipelineVertexInputStateCreateInfo vertex_input_info = {};
	vertex_input_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertex_input_info.vertexBindingDescriptionCount = 0;
	vertex_input_info.pVertexBindingDescriptions = nullptr;
	vertex_input_info.vertexAttributeDescriptionCount = 0;
	vertex_input_info.pVertexAttributeDescriptions = nullptr;

	VkPipelineInputAssemblyStateCreateInfo input_assembly = {};
	input_assembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	input_assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	input_assembly.primitiveRestartEnable = VK_FALSE;

	VkViewport viewport = {};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = (float) init.swapchain.extent.width;
	viewport.height = (float) init.swapchain.extent.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	VkRect2D scissor = {};
	scissor.offset = {0, 0};
	scissor.extent = init.swapchain.extent;

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
	rasterizer.cullMode = VK_CULL_MODE_NONE;
	rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rasterizer.depthBiasEnable = VK_FALSE;

	VkPipelineMultisampleStateCreateInfo multisampling = {};
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

	VkPipelineColorBlendAttachmentState color_blend_attachment = {};
	color_blend_attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	color_blend_attachment.blendEnable = VK_FALSE;

	VkPipelineColorBlendStateCreateInfo color_blending = {};
	color_blending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	color_blending.logicOpEnable = VK_FALSE;
	color_blending.logicOp = VK_LOGIC_OP_COPY;
	color_blending.attachmentCount = 1;
	color_blending.pAttachments = &color_blend_attachment;

	VkPipelineDepthStencilStateCreateInfo depth_stencil = {};
	depth_stencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depth_stencil.depthTestEnable = VK_TRUE;
	depth_stencil.depthWriteEnable = VK_FALSE;
	depth_stencil.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
	depth_stencil.depthBoundsTestEnable = VK_FALSE;
	depth_stencil.stencilTestEnable = VK_FALSE;

	VkPipelineRenderingCreateInfoKHR pipeline_rendering_create_info = {};
	pipeline_rendering_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
	pipeline_rendering_create_info.colorAttachmentCount = 1;
	pipeline_rendering_create_info.pColorAttachmentFormats = &init.swapchain.image_format;
	pipeline_rendering_create_info.depthAttachmentFormat = VK_FORMAT_D16_UNORM;

	VkGraphicsPipelineCreateInfo pipeline_info = {};
	pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipeline_info.stageCount = 2;
	pipeline_info.pStages = shader_stages;
	pipeline_info.pVertexInputState = &vertex_input_info;
	pipeline_info.pInputAssemblyState = &input_assembly;
	pipeline_info.pViewportState = &viewport_state;
	pipeline_info.pRasterizationState = &rasterizer;
	pipeline_info.pMultisampleState = &multisampling;
	pipeline_info.pColorBlendState = &color_blending;
	pipeline_info.layout = pipeline_layout;
	pipeline_info.renderPass = nullptr;
	pipeline_info.pNext = &pipeline_rendering_create_info;
	pipeline_info.subpass = 0;
	pipeline_info.basePipelineHandle = VK_NULL_HANDLE;
	pipeline_info.pDepthStencilState = &depth_stencil;

	VkPipeline pipeline;

	if (vkCreateGraphicsPipelines(init.device, VK_NULL_HANDLE, 1, &pipeline_info, nullptr, &pipeline) != VK_SUCCESS) {
		throw std::runtime_error("failed to create graphics pipeline!");
	}

	vkDestroyShaderModule(init.device, vert_shader_module, nullptr);
	vkDestroyShaderModule(init.device, frag_shader_stage_info.module, nullptr);


	return pipeline;
}

void render_cubemap(
	const Init& init,
	const RenderData& render_data,
	const CubeMap& cube_map,
	VkCommandBuffer command_buffer,
	uint32_t image_index)
{

	VkRenderingAttachmentInfo color_attachments[1];
	color_attachments[0].sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
	color_attachments[0].pNext = nullptr;
	color_attachments[0].imageView = render_data.swapchain_image_views[image_index];
	color_attachments[0].imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	color_attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	color_attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	color_attachments[0].clearValue = {0.0f, 0.0f, 0.0f, 1.0f};
	color_attachments[0].resolveMode = VK_RESOLVE_MODE_NONE;
	color_attachments[0].resolveImageView = VK_NULL_HANDLE;
	color_attachments[0].resolveImageLayout = VK_IMAGE_LAYOUT_UNDEFINED;

	VkRenderingAttachmentInfo depth_attachment;
	depth_attachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
	depth_attachment.pNext = nullptr;
	depth_attachment.imageView = render_data.depth_image.imageView;
	depth_attachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	depth_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depth_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	depth_attachment.clearValue = {1.0f, 0};
	depth_attachment.resolveMode = VK_RESOLVE_MODE_NONE;
	depth_attachment.resolveImageView = VK_NULL_HANDLE;
	depth_attachment.resolveImageLayout = VK_IMAGE_LAYOUT_UNDEFINED;

	VkRenderingInfo rendering_info;
	rendering_info.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
	rendering_info.pNext = nullptr;
	rendering_info.flags = 0;
	rendering_info.renderArea.offset = {0, 0};
	rendering_info.renderArea.extent = init.swapchain.extent;
	rendering_info.layerCount = 1;
	rendering_info.viewMask = 0;
	rendering_info.colorAttachmentCount = 1;
	rendering_info.pColorAttachments = color_attachments;
	rendering_info.pDepthAttachment = &depth_attachment;
	rendering_info.pStencilAttachment = nullptr;

	init.disp.cmdBeginRendering(command_buffer, &rendering_info);

	VkViewport viewport = {};
	viewport.width = static_cast<float>(init.swapchain.extent.width);
	viewport.height = static_cast<float>(init.swapchain.extent.height);
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	VkRect2D scissor = {};
	scissor.extent = init.swapchain.extent;

	init.disp.cmdSetViewport(command_buffer, 0, 1, &viewport);
	init.disp.cmdSetScissor(command_buffer, 0, 1, &scissor);
	vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, cube_map.pipeline);

	init.disp.cmdBindDescriptorSets(command_buffer,
									VK_PIPELINE_BIND_POINT_GRAPHICS,
									render_data.cube_map.pipeline_layout, 0, 1,
									&render_data.descriptor_sets[image_index], 0, nullptr);

	init.disp.cmdDraw(command_buffer, 36, 1, 0, 0);
	init.disp.cmdEndRendering(command_buffer);

}

}