#pragma once

#include "camera.hpp"
#include "shadow.hpp"

struct Init {
    GLFWwindow* window;
    vkb::Instance instance;
    vkb::InstanceDispatchTable inst_disp;
    VkSurfaceKHR surface;
    vkb::PhysicalDevice physical_device;
    vkb::Device device;
    vkb::DispatchTable disp;
    vkb::Swapchain swapchain;
    VmaAllocator allocator;
    VkQueue graphics_queue;
    VkQueue present_queue;
    VkCommandPool command_pool;
};

struct BufferAllocation {
    VkBuffer buffer;
    VmaAllocation allocation;
    VkDeviceSize size;
};

struct DescriptorSetLayouts {
    VkDescriptorSetLayout object;
    VkDescriptorSetLayout camera;
    VkDescriptorSetLayout material;
    VkDescriptorSetLayout light;
};

struct DescriptorSets {
    VkDescriptorSet object;
    VkDescriptorSet camera;
    VkDescriptorSet material;
    VkDescriptorSet light;
};

struct RenderData {
    VkQueue graphics_queue;
    VkQueue present_queue;

    std::vector<VkImage> swapchain_images;
    std::vector<VkImageView> swapchain_image_views;
    std::vector<VkFramebuffer> framebuffers;

    VkRenderPass render_pass;
    VkPipelineLayout pipeline_layout;
    VkPipeline graphics_pipeline;

    VkCommandPool command_pool;

    std::vector<VkCommandBuffer> command_buffers;

    std::vector<VkSemaphore> available_semaphores;
    std::vector<VkSemaphore> finished_semaphore;
    std::vector<VkFence> in_flight_fences;
    std::vector<VkFence> image_in_flight;
    size_t current_frame = 0;

    BufferAllocation vertex_buffer;
    BufferAllocation index_buffer;

    BufferAllocation objectBuffer;
    BufferAllocation cameraBuffer;

    VkDescriptorPool descriptor_pool;

    DescriptorSetLayouts descriptorSetLayouts;
    std::vector<DescriptorSets> descriptorSets;

    Camera camera;

    struct {
        VkImage image;
        VmaAllocation allocation;
        VkImageView view;
    } depth_image;

    VkImageView depth_image_view;

    ShadowMap shadow_map;
    ShadowPipeline shadow_pipeline;

};

struct Vertex {
    glm::vec3 pos;
    glm::vec3 color;
};

struct InstanceData {
    glm::mat4 model;
};

struct InstancedRenderData {
    BufferAllocation vertex_buffer;
    BufferAllocation instance_buffer;
    uint32_t vertex_count;
    uint32_t instance_count;
    uint32_t max_instance_count;
    VkDescriptorSetLayout descriptor_set_layout;
    VkDescriptorPool descriptor_pool;
    VkDescriptorSet descriptor_set;
    VkPipelineLayout pipeline_layout;
};


struct UniformBufferObject {
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 proj;
};

struct ObjectUbo {
    glm::mat4 model;
};

struct CameraUbo {
    glm::mat4 view;
    glm::mat4 proj;
};

struct MaterialUbo {
    glm::vec4 ambient;
    glm::vec4 diffuse;
    glm::vec4 specular;
    float shininess;
};


