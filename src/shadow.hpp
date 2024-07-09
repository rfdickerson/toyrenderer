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


VkResult createShadowMap(Init &init, RenderData& data, ShadowMap& shadowMap, uint32_t width, uint32_t height);
void destroyShadowMap(ShadowMap& shadowMap);

VkResult beginShadowRenderPass(VkCommandBuffer commandBuffer, ShadowMap& shadowMap);
void endShadowRenderPass(VkCommandBuffer commandBuffer);

VkResult createShadowMappingPipeline(Init& init, RenderData& data, ShadowPipeline& shadowPipeline);
void destroyShadowMappingPipeline(Init& init, ShadowPipeline& shadowPipeline);
