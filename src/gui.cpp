#include "gui.hpp"

#include "common.hpp"
#include "debug_utils.hpp"

// imgui includes
#include "../extern/imgui/imgui.h"
#include "../extern/imgui/imgui_impl_glfw.h"
#include "../extern/imgui/imgui_impl_vulkan.h"

#include <spdlog/spdlog.h>

namespace obsidian {

int create_imgui(Init& init, RenderData& data) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();

    VkPipelineRenderingCreateInfo pipeline_rendering_create_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
        .pNext = nullptr,
        .viewMask = 0,
        .colorAttachmentCount = 1,
        .pColorAttachmentFormats = &init.swapchain.image_format,
        .depthAttachmentFormat = VK_FORMAT_D16_UNORM,
        .stencilAttachmentFormat = VK_FORMAT_UNDEFINED,
    };

    ImGui_ImplGlfw_InitForVulkan(init.window, true);

    ImGui_ImplVulkan_InitInfo init_info = {
        .Instance = init.instance.instance,
        .PhysicalDevice = init.physical_device.physical_device,
        .Device = init.device.device,
        .QueueFamily = init.device.get_queue_index(vkb::QueueType::graphics).value(), 
        .Queue = init.graphics_queue,
        .DescriptorPool = data.descriptor_pool,
        .RenderPass = nullptr,
        .MinImageCount = init.swapchain.image_count,
        .ImageCount = init.swapchain.image_count,
        .MSAASamples = VK_SAMPLE_COUNT_1_BIT,
        .PipelineCache = VK_NULL_HANDLE,
        .UseDynamicRendering = VK_TRUE,
        .PipelineRenderingCreateInfo = pipeline_rendering_create_info,

        // optional
        .Allocator = nullptr,
        .CheckVkResultFn = nullptr,
    };

    ImGui_ImplVulkan_Init(&init_info);
    spdlog::info("ImGui initialized");
    ImGui_ImplVulkan_CreateFontsTexture();
    spdlog::info("ImGui font texture created");

    return 0;
}

void render_imgui(Init& init, const VkCommandBuffer& command_buffer, const VkImageView& image_view) {

	std::array<VkRenderingAttachmentInfo, 1> colorAttachments = {{
    {
        .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
        .pNext = nullptr,
        .imageView = image_view,
        .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
	    .resolveMode = VK_RESOLVE_MODE_NONE,
	    .resolveImageView = nullptr,
	    .resolveImageLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .loadOp = VK_ATTACHMENT_LOAD_OP_LOAD,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        .clearValue = {}, // Add this line if needed, or set to your desired clear value
    }
	}};

	const VkRenderingInfo renderingInfo = {
		.sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
		.pNext = nullptr,
		.flags = 0,
	    .renderArea = {0, 0, init.swapchain.extent.width, init.swapchain.extent.height},
	    .layerCount = 1,
	    .viewMask = 0,
	    .colorAttachmentCount = colorAttachments.size(),
		.pColorAttachments = colorAttachments.data(),
		.pDepthAttachment = nullptr,
		.pStencilAttachment = nullptr,
	};

	begin_debug_label(init, command_buffer, "ImGui Rendering", {0.5f, 0.76f, 0.34f});
	init.disp.cmdBeginRendering(command_buffer, &renderingInfo);
	ImGui::Render();
	ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), command_buffer);
	init.disp.cmdEndRendering(command_buffer);
	end_debug_label(init, command_buffer);
}

void destroy_imgui() {
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    spdlog::info("ImGui destroyed");
}

int create_new_imgui_frame(Init& init, RenderData& render_data) {
	ImGui_ImplVulkan_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	ImGui::Begin("Obsidian Engine");

	// show light direction and change it
	ImGui::SliderFloat3("Light Direction", &render_data.shadow_map.light_direction.x, -1.0f, 1.0f);

	ImGui::SliderFloat("Light Distance", &render_data.shadow_map.light_distance, 10.0f, 100.0f);
	ImGui::SliderFloat("Light Volume", &render_data.shadow_map.radius, 1.0f, 100.0f);
	ImGui::SliderFloat("Light Near Plane", &render_data.shadow_map.near_plane, 0.1f, 50.0f);
	ImGui::SliderFloat("Light Far Plane", &render_data.shadow_map.far_plane, 5.0f, 100.0f);
	ImGui::SliderFloat("Depth Bias Constant", &render_data.shadow_map.bias, 0.0f, 2.0f);
	ImGui::SliderFloat("Depth Bias Slope", &render_data.shadow_map.slope_bias, 0.0f, 2.0f);

	// show camera coordinates
	ImGui::Text("Camera position: %.2f %.2f %.2f", render_data.camera->position.x, render_data.camera->position.y, render_data.camera->position.z);

	// show camera facing
	ImGui::Text("Camera facing: %.2f %.2f %.2f", render_data.camera->front.x, render_data.camera->front.y, render_data.camera->front.z);

	ImGui::End();

	return 0;
}

}