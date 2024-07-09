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

// Light struct for uniform buffer object
// 64 bytes
struct Light {
    glm::vec4 position; // 16
    glm::vec4 color; // 16
    glm::vec4 direction; // 16
    float radius; // for attenuation calculations // 4
    float innerCutoff; // for spot lights 4
    float outerCutoff; // for spot lights 4
    float padding; // to ensure 16 byte alignment 4
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
    BufferAllocation materialBuffer;
    BufferAllocation lightsBuffer;
    BufferAllocation shadowsBuffer;

    VkDescriptorPool descriptor_pool;

    DescriptorSetLayouts descriptorSetLayouts;
    std::vector<DescriptorSets> descriptorSets;

    Camera camera;
    std::vector<Light> lights;

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
    alignas(16) glm::vec3 pos;
    alignas(16) glm::vec3 color;
    alignas(16) glm::vec3 normal;
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


struct LightsUbo {
    std::array<Light, 8> lights;
    int numActiveLights;
    glm::vec3 padding;
};

struct ShadowsUBO {
    glm::mat4 lightSpaceMatrices[8];
    float cascadeSplits[4];
    int numActiveShadowMaps;
    glm::vec3 padding;
};


