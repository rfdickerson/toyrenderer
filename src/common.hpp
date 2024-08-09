#pragma once

#include "camera.hpp"
#include "image_loader.hpp"

namespace obsidian
{

class CubeMap;
struct Mesh;
struct ShadowMap;

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

struct BufferAllocation
{
	VkBuffer      buffer;
	VmaAllocation allocation;
	VkDeviceSize  size;
};

struct ShadowMap {
	VkImage image;
	VkImageView image_view;
	VmaAllocation allocation;
	VkSampler sampler;
	glm::mat4 light_space_matrix;
	glm::vec3 light_direction;
	float light_distance = 25.0f;
	float bias = 1.25f;
	float slope_bias = 1.75f;
	float radius = 10.0f;
	float near_plane = 1.0f;
	float far_plane = 25.0f;
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

	Camera camera;
	TextureImage    texture;
	TextureImage    cube_map_texture;
	CubeMap         *cube_map;
	Mesh 		   	*mesh;
	Mesh 		   	*plane_mesh;

	// shadow stuff
	ShadowMap 	   			shadow_map;
	VkPipelineLayout 		shadow_pipeline_layout;
	VkPipeline 		   		shadow_pipeline;

	BufferAllocation staging_buffer;

	struct
	{
		VkImage       image;
		VmaAllocation allocation;
		VkImageView   view;
	} depth_image;

	VkImageView depth_image_view;
	float       lastX      = 400;
	float       lastY      = 300;
	bool        firstMouse = true;
};

struct Vertex
{
	glm::vec3 pos;
	glm::vec3 color;
	glm::vec2 tex_coord;
	glm::vec3 normal;
};

struct UniformBufferObject
{
	glm::mat4 model;
	glm::mat4 view;
	glm::mat4 proj;
	glm::mat4 lightSpaceMatrix;
	glm::vec3 lightDirection;
	float nearPlane;
	float farPlane;
	float padding;
};



}; // namespace obsidian

