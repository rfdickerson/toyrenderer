#pragma once

namespace obsidian {

// forward declarations
struct Init;
struct RenderData;

int create_imgui(Init& init, RenderData& data);

void render_imgui(Init& init, const VkCommandBuffer& command_buffer, const VkImageView& image_view);

void destroy_imgui();

int create_new_imgui_frame(Init& init, RenderData& render_data);

}