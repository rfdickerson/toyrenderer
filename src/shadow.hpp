#pragma once

struct Init;
struct RenderData;

struct ShadowMap {
    VkDevice device;
    VmaAllocator allocator;
    uint32_t width;
    uint32_t height;

    VkImage image;
    VmaAllocation imageAllocation;
    VkImageView view;
    VkSampler sampler;
    VkRenderPass renderPass;
    VkFramebuffer framebuffer;
};

struct ShadowPipeline {
    VkPipeline pipeline;
    VkPipelineLayout layout;
};

struct ShadowUniformBufferObject {
    glm::mat4 lightSpaceMatrix;
    glm::mat4 model;
};

VkResult createShadowMap(Init &init, RenderData& data, ShadowMap& shadowMap, uint32_t width, uint32_t height);
void destroyShadowMap(ShadowMap& shadowMap);

VkResult beginShadowRenderPass(VkCommandBuffer commandBuffer, ShadowMap& shadowMap);
void endShadowRenderPass(VkCommandBuffer commandBuffer);

VkResult createShadowMappingPipeline(Init& init, RenderData& data, ShadowPipeline& shadowPipeline);
void destroyShadowMappingPipeline(Init& init, ShadowPipeline& shadowPipeline);

VkResult createShadowMappingDescriptorSetLayout(Init& init, RenderData& data, ShadowPipeline& shadowPipeline);

std::vector<VkDescriptorSet> createShadowMappingDescriptorSets(Init& init, RenderData& data, ShadowPipeline& shadowPipeline, uint32_t framesInFlight);

void updateShadowMappingDescriptorSets(Init& init, RenderData& data, ShadowPipeline& shadowPipeline, uint32_t currentImageIndex);