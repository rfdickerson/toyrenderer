//
// Created by rfdic on 7/13/2024.
//

#ifndef TOYRENDERER_CUBE_MAP_HPP
#define TOYRENDERER_CUBE_MAP_HPP

class Init;
class RenderData;

class CubeMap {
public:
    CubeMap(Init& init, RenderData& renderData);
    ~CubeMap();

    Init &init;
    RenderData &renderData;

    VkPipeline pipeline;
    VkPipelineLayout pipeline_layout;

    VkRenderPass render_pass;

    VkResult create_render_pass();
    VkResult create_pipeline();
    VkResult create_pipeline_layout();
};


#endif //TOYRENDERER_CUBE_MAP_HPP
