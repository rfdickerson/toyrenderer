#pragma once

#include "buffer.hpp"
#include "camera.hpp"
#include "cube_map.hpp"
#include "image_loader.hpp"
#include "mesh.hpp"
#include "shadow.hpp"
#include "image.hpp"

namespace obsidian
{

constexpr size_t MAX_BUFFERS = 200;

struct Init
{
	GLFWwindow                *window;
	vkb::Instance              instance;
	vkb::InstanceDispatchTable inst_disp;
	VkSurfaceKHR               surface;
	vkb::PhysicalDevice        physical_device;
	vkb::Device                device;
	vkb::DispatchTable         disp;
	vkb::Swapchain             swapchain;
	VmaAllocator               allocator;
	VkQueue                    graphics_queue;
	VkCommandPool              command_pool;
};

struct RenderData
{

	std::vector<VkImage>       swapchain_images;
	std::vector<VkImageView>   swapchain_image_views;
	std::vector<VkFramebuffer> framebuffers;

	VkRenderPass     render_pass;
	VkPipelineLayout pipeline_layout;
	VkPipeline       graphics_pipeline;

	VkCommandPool command_pool;

	std::vector<VkCommandBuffer> command_buffers;

	std::vector<VkSemaphore> available_semaphores;
	std::vector<VkSemaphore> finished_semaphore;
	std::vector<VkFence>     in_flight_fences;
	std::vector<VkFence>     image_in_flight;
	size_t                   current_frame = 0;

	std::vector<BufferAllocation> uniform_buffers;

	VkDescriptorPool             descriptor_pool;
	VkDescriptorSetLayout        descriptor_set_layout;
	std::vector<VkDescriptorSet> descriptor_sets;

	Camera* camera;
	TextureImage    texture;
	TextureImage    cube_map_texture;

	std::array<BufferAllocation, MAX_BUFFERS> vertex_buffers;
	std::array<BufferAllocation, MAX_BUFFERS> index_buffers;

	CubeMap         cube_map;

	std::vector<Mesh> meshes;

	// shadow stuff
	ShadowMap 	   			shadow_map;
	VkPipelineLayout 		shadow_pipeline_layout;
	VkPipeline 		   		shadow_pipeline;

	BufferAllocation staging_buffer;
	Image depth_image;


	float       lastX      = 400;
	float       lastY      = 300;
	bool        firstMouse = true;
};






}; // namespace obsidian

