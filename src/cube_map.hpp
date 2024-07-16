#pragma once

struct Init;
struct RenderData;

class CubeMap {
public:
    CubeMap(Init& init, RenderData& renderData);
    ~CubeMap();

    Init &init;
    RenderData &renderData;

    VkPipeline pipeline;
    VkPipelineLayout pipeline_layout;

    VkRenderPass render_pass;

    VkResult createRenderPass();
    VkResult createPipeline();
    VkResult createPipelineLayout();
};

