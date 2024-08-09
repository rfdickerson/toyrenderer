//
// Created by rfdic on 8/4/2024.
//

#ifndef TOYRENDERER_SHADOW_HPP
#define TOYRENDERER_SHADOW_HPP

namespace obsidian
{
struct Init;
struct RenderData;



void draw_shadow(Init &init, RenderData &data, VkCommandBuffer &command_buffer, uint32_t image_index);

void update_shadow(Init &init, RenderData &data);

// create the shadow map
void init_shadow_map(Init &init, RenderData &data);
void create_shadow_map(Init &init, RenderData& data, uint32_t width, uint32_t height);
void cleanup_shadow_map(Init &init, RenderData &data);

// create the shadow pipeline
void init_shadow_pipeline(Init &init, RenderData &data);


}

#endif        // TOYRENDERER_SHADOW_HPP
