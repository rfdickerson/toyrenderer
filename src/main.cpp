#include "common.hpp"
#include "utils.hpp"


#include <spdlog/spdlog.h>
#include <iostream>
#include <GLFW/glfw3.h>

#include "camera.hpp"
#include "cube_map.hpp"
#include "debug_utils.hpp"
#include "image.hpp"
#include "image_loader.hpp"
#include "mesh.hpp"
#include "obj_loader.hpp"
#include "shadow.hpp"
#include "uniforms.hpp"
#include "gui.hpp"

#include "../extern/imgui/imgui_impl_glfw.h"

using namespace obsidian;

const int WIDTH = 1280;
const int HEIGHT = 720;

const int MAX_FRAMES_IN_FLIGHT = 2;


void mouse_callback(GLFWwindow *window, double xpos, double ypos) {
    auto data = static_cast<RenderData *>(glfwGetWindowUserPointer(window));

	// if (ImGui::GetIO().WantCaptureMouse) {
	// 	// ImGui is using the mouse, don't process the input for your camera
	// 	return;
	// }

    if (data->firstMouse) {
        data->lastX = xpos;
        data->lastY = ypos;
        data->firstMouse = false;
    }

    double xoffset = xpos - data->lastX;
    double yoffset = data->lastY - ypos; // reversed since y-coordinates go from bottom to top
    data->lastX = xpos;
    data->lastY = ypos;

	data->camera->process_mouse_movement(xoffset, yoffset);

	ImGui_ImplGlfw_CursorPosCallback(window, xpos, ypos);
}


void update_uniform_buffer(uint32_t current, Init &init, RenderData& renderData) {

	UniformBufferObject ubo {
		.model = glm::mat4(1.0f),
		.view = renderData.camera->getViewMatrix(),
		.proj = renderData.camera->getProjectionMatrix(),
	    .lightSpaceMatrix = renderData.shadow_map.light_space_matrix,
	    .lightDirection = renderData.shadow_map.light_direction
	};

	copy_buffer_data(init, renderData.uniform_buffers[current], 0, sizeof(ubo), &ubo);
}

GLFWwindow* create_window_glfw(const char* window_name = "", bool resize = true) {
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    if (!resize) glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    return glfwCreateWindow(WIDTH, HEIGHT, window_name, nullptr, nullptr);
}

void destroy_window_glfw(GLFWwindow* window) {
    glfwDestroyWindow(window);
    glfwTerminate();
}

VkSurfaceKHR create_surface_glfw(VkInstance instance, GLFWwindow* window, VkAllocationCallbacks* allocator = nullptr) {
    VkSurfaceKHR surface = VK_NULL_HANDLE;

    VkResult err = glfwCreateWindowSurface(instance, window, allocator, &surface);
    if (err) {
        const char* error_msg;
        int ret = glfwGetError(&error_msg);
        if (ret != 0) {
            std::cout << ret << " ";
            if (error_msg != nullptr) std::cout << error_msg;
            std::cout << "\n";
        }
        surface = VK_NULL_HANDLE;
    }
    return surface;
}

int device_initialization(Init& init) {
    init.window = create_window_glfw("Obsidian", true);

    vkb::InstanceBuilder instance_builder;

    auto instance_ret = instance_builder
            .set_app_name("Vulkan Triangle")
            .set_engine_name("AwesomeEngine")
			.require_api_version(1, 3, 0)
            .request_validation_layers()
            .use_default_debug_messenger()
            .build();

    if (!instance_ret) {
        std::cout << instance_ret.error().message() << "\n";
        return -1;
    }
    init.instance = instance_ret.value();

    init.inst_disp = init.instance.make_table();

    init.surface = create_surface_glfw(init.instance, init.window);

    VkPhysicalDeviceFeatures required_features = {};
    required_features.samplerAnisotropy = VK_TRUE;
	required_features.textureCompressionBC = VK_TRUE;

	VkPhysicalDeviceVulkan13Features vulkan13Features = {};
	vulkan13Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
	vulkan13Features.maintenance4 = VK_TRUE;
	vulkan13Features.synchronization2 = VK_TRUE;
	vulkan13Features.dynamicRendering = VK_TRUE;

	VkPhysicalDeviceVulkan12Features features12 = {};
	features12.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
	features12.bufferDeviceAddress = VK_TRUE;
	features12.descriptorIndexing = true;

	VkPhysicalDeviceDynamicRenderingFeatures dynamic_rendering_features = {};
	dynamic_rendering_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES;
	dynamic_rendering_features.dynamicRendering = VK_TRUE;

    vkb::PhysicalDeviceSelector phys_device_selector(init.instance);

    auto phys_device_ret = phys_device_selector
	                           .set_required_features(required_features)
	                           .set_required_features_12(features12)
	                           .set_required_features_13(vulkan13Features)
	                           .set_surface(init.surface).select();
    if (!phys_device_ret) {
        std::cout << phys_device_ret.error().message() << "\n";
        return -1;
    }
    vkb::PhysicalDevice physical_device = phys_device_ret.value();
    init.physical_device = physical_device;

    vkb::DeviceBuilder device_builder{ physical_device };
    auto device_ret = device_builder.build();
    if (!device_ret) {
        std::cout << device_ret.error().message() << "\n";
        return -1;
    }
    init.device = device_ret.value();

    init.disp = init.device.make_table();

    // get graphics queue
    auto graphics_queue_ret = init.device.get_queue(vkb::QueueType::graphics);
    if (!graphics_queue_ret) {
        std::cout << graphics_queue_ret.error().message() << "\n";
        return -1;
    }
    init.graphics_queue = graphics_queue_ret.value();

    VkCommandPoolCreateInfo pool_info = {};
    pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    pool_info.queueFamilyIndex = init.device.get_queue_index(vkb::QueueType::graphics).value();
    pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

    if (init.disp.createCommandPool(&pool_info, nullptr, &init.command_pool) != VK_SUCCESS) {
        std::cout << "failed to create command pool\n";
        return -1;
    }

	VmaVulkanFunctions vulkanFunctions = {};
	vulkanFunctions.vkGetInstanceProcAddr = init.inst_disp.fp_vkGetInstanceProcAddr;
	vulkanFunctions.vkGetDeviceProcAddr = init.device.fp_vkGetDeviceProcAddr;

    VmaAllocatorCreateInfo allocatorInfo = {};
    allocatorInfo.physicalDevice = physical_device.physical_device;
    allocatorInfo.device = init.device.device;
    allocatorInfo.instance = init.instance.instance;
	allocatorInfo.vulkanApiVersion = VK_API_VERSION_1_3;
	allocatorInfo.pVulkanFunctions = &vulkanFunctions;

    if (vmaCreateAllocator(&allocatorInfo, &init.allocator) != VK_SUCCESS) {
        std::cout << "failed to create VMA allocator\n";
        return -1;
    }

    return 0;
}

int create_swapchain(Init& init) {

    vkb::SwapchainBuilder swapchain_builder{ init.device };

    auto swap_ret = swapchain_builder.set_old_swapchain(init.swapchain).build();
    if (!swap_ret) {
        std::cout << swap_ret.error().message() << " " << swap_ret.vk_result() << "\n";
        return -1;
    }
    vkb::destroy_swapchain(init.swapchain);
    init.swapchain = swap_ret.value();
    return 0;
}

int create_graphics_pipeline(Init& init, RenderData& data) {
    auto vert_code = read_file("shaders/simple.vert.spv");
    auto frag_code = read_file("shaders/simple.frag.spv");

    VkShaderModule vert_module = create_shader_module(init, vert_code);
    VkShaderModule frag_module = create_shader_module(init, frag_code);
    if (vert_module == VK_NULL_HANDLE || frag_module == VK_NULL_HANDLE) {
        std::cout << "failed to create shader module\n";
        return -1; // failed to create shader modules
    }

    VkPipelineShaderStageCreateInfo vert_stage_info = {};
    vert_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vert_stage_info.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vert_stage_info.module = vert_module;
    vert_stage_info.pName = "main";

    VkPipelineShaderStageCreateInfo frag_stage_info = {};
    frag_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    frag_stage_info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    frag_stage_info.module = frag_module;
    frag_stage_info.pName = "main";

    VkPipelineShaderStageCreateInfo shader_stages[] = { vert_stage_info, frag_stage_info };

    VkVertexInputBindingDescription bindingDescription;
    bindingDescription.binding = 0;
    bindingDescription.stride = sizeof(Vertex);
    bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    std::array<VkVertexInputAttributeDescription, 6> attributeDescriptions{};

    attributeDescriptions[0].binding = 0;
    attributeDescriptions[0].location = 0;
    attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[0].offset = offsetof(Vertex, pos);

    attributeDescriptions[1].binding = 0;
    attributeDescriptions[1].location = 1;
    attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[1].offset = offsetof(Vertex, color);

    attributeDescriptions[2].binding = 0;
    attributeDescriptions[2].location = 2;
    attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
    attributeDescriptions[2].offset = offsetof(Vertex, tex_coord);

    attributeDescriptions[3].binding = 0;
    attributeDescriptions[3].location = 3;
    attributeDescriptions[3].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[3].offset = offsetof(Vertex, normal);

    attributeDescriptions[4].binding = 0;
	attributeDescriptions[4].location = 4;
	attributeDescriptions[4].format   = VK_FORMAT_R32G32B32_SFLOAT;
	attributeDescriptions[4].offset   = offsetof(Vertex, tangent);

    attributeDescriptions[5].binding = 0;
	attributeDescriptions[5].location = 5;
	attributeDescriptions[5].format   = VK_FORMAT_R32G32B32_SFLOAT;
	attributeDescriptions[5].offset   = offsetof(Vertex, bitangent);


    VkPipelineVertexInputStateCreateInfo vertex_input_info = {};
    vertex_input_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertex_input_info.vertexBindingDescriptionCount = 1;
    vertex_input_info.pVertexBindingDescriptions = &bindingDescription;
    vertex_input_info.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
    vertex_input_info.pVertexAttributeDescriptions = attributeDescriptions.data();

    VkPipelineInputAssemblyStateCreateInfo input_assembly = {};
    input_assembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    input_assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    input_assembly.primitiveRestartEnable = VK_FALSE;


    VkViewport viewport = {};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float)init.swapchain.extent.width;
    viewport.height = (float)init.swapchain.extent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor = {};
    scissor.offset = { 0, 0 };
    scissor.extent = init.swapchain.extent;

    VkPipelineViewportStateCreateInfo viewport_state = {};
    viewport_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewport_state.viewportCount = 1;
    viewport_state.pViewports = &viewport;
    viewport_state.scissorCount = 1;
    viewport_state.pScissors = &scissor;

    VkPipelineRasterizationStateCreateInfo rasterizer = {};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;

    VkPipelineMultisampleStateCreateInfo multisampling = {};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
    colorBlendAttachment.colorWriteMask =
            VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE;

    VkPipelineColorBlendStateCreateInfo color_blending = {};
    color_blending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    color_blending.logicOpEnable = VK_FALSE;
    color_blending.logicOp = VK_LOGIC_OP_COPY;
    color_blending.attachmentCount = 1;
    color_blending.pAttachments = &colorBlendAttachment;
    color_blending.blendConstants[0] = 0.0f;
    color_blending.blendConstants[1] = 0.0f;
    color_blending.blendConstants[2] = 0.0f;
    color_blending.blendConstants[3] = 0.0f;

	VkPushConstantRange pushConstantRange = {};
	pushConstantRange.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	pushConstantRange.offset = 0;
	pushConstantRange.size = sizeof(PushConstantBuffer);

    VkPipelineLayoutCreateInfo pipeline_layout_info = {};
    pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipeline_layout_info.setLayoutCount = 1;
    pipeline_layout_info.pSetLayouts = &data.descriptor_set_layout;
    pipeline_layout_info.pushConstantRangeCount = 1;
	pipeline_layout_info.pPushConstantRanges = &pushConstantRange;

    if (init.disp.createPipelineLayout(&pipeline_layout_info, nullptr, &data.pipeline_layout) != VK_SUCCESS) {
        std::cout << "failed to create pipeline layout\n";
        return -1; // failed to create pipeline layout
    }

    std::vector<VkDynamicState> dynamic_states = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };

    VkPipelineDynamicStateCreateInfo dynamic_info = {};
    dynamic_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamic_info.dynamicStateCount = static_cast<uint32_t>(dynamic_states.size());
    dynamic_info.pDynamicStates = dynamic_states.data();

    VkPipelineDepthStencilStateCreateInfo depthStencil{};
    depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencil.depthTestEnable = VK_TRUE;
    depthStencil.depthWriteEnable = VK_TRUE;
    depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
    depthStencil.depthBoundsTestEnable = VK_FALSE;
    depthStencil.stencilTestEnable = VK_FALSE;

	// set up dynamic rendering
	VkPipelineRenderingCreateInfo renderingCreateInfo = {};
	renderingCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
	renderingCreateInfo.pNext = nullptr;
	renderingCreateInfo.colorAttachmentCount = 1;
	renderingCreateInfo.pColorAttachmentFormats = &init.swapchain.image_format;
	renderingCreateInfo.depthAttachmentFormat = data.depth_image.format;

    VkGraphicsPipelineCreateInfo pipeline_info = {};
    pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipeline_info.stageCount = 2;
    pipeline_info.pStages = shader_stages;
    pipeline_info.pVertexInputState = &vertex_input_info;
    pipeline_info.pInputAssemblyState = &input_assembly;
    pipeline_info.pViewportState = &viewport_state;
    pipeline_info.pRasterizationState = &rasterizer;
    pipeline_info.pMultisampleState = &multisampling;
    pipeline_info.pColorBlendState = &color_blending;
    pipeline_info.pDynamicState = &dynamic_info;
    pipeline_info.layout = data.pipeline_layout;
    pipeline_info.renderPass = nullptr;
    pipeline_info.subpass = 0;
    pipeline_info.basePipelineHandle = VK_NULL_HANDLE;
    pipeline_info.pDepthStencilState = &depthStencil;
	pipeline_info.pNext = &renderingCreateInfo;

    if (init.disp.createGraphicsPipelines(VK_NULL_HANDLE, 1, &pipeline_info, nullptr, &data.graphics_pipeline) != VK_SUCCESS) {
        throw std::runtime_error("failed to create pipeline");
    }

    init.disp.destroyShaderModule(frag_module, nullptr);
    init.disp.destroyShaderModule(vert_module, nullptr);
    return 0;
}

int create_command_pool(Init& init, RenderData& data) {
    VkCommandPoolCreateInfo pool_info = {};
    pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    pool_info.queueFamilyIndex = init.device.get_queue_index(vkb::QueueType::graphics).value();
    pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

    if (init.disp.createCommandPool(&pool_info, nullptr, &data.command_pool) != VK_SUCCESS) {
        std::cout << "failed to create command pool\n";
        return -1; // failed to create command pool
    }

    // let's create a main command pool
    VkCommandPoolCreateInfo main_pool_info = {};
    main_pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    main_pool_info.queueFamilyIndex = init.device.get_queue_index(vkb::QueueType::graphics).value();
    main_pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

    if (init.disp.createCommandPool(&main_pool_info, nullptr, &init.command_pool) != VK_SUCCESS) {
        std::cout << "failed to create main command pool\n";
        return -1; // failed to create command pool
    }

    return 0;
}

int create_command_buffers(Init& init, RenderData& data) {
    data.command_buffers.resize(init.swapchain.image_count);

    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = data.command_pool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = (uint32_t)data.command_buffers.size(); // We only need one command buffer now

    if (init.disp.allocateCommandBuffers(&allocInfo, data.command_buffers.data()) != VK_SUCCESS) {
        std::cout << "failed to allocate command buffers\n";
        return -1;
    }
    return 0;
}



void begin_rendering(Init& init,
                     const VkCommandBuffer command_buffer,
                     const VkImageView& image_view,
                     const VkImageView& depth_image_view) {

	VkRenderingAttachmentInfo color_attachments[1];
	color_attachments[0].sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
	color_attachments[0].clearValue.color = {0.0f, 0.0f, 0.0f, 1.0f};
	color_attachments[0].clearValue.depthStencil = {1.0f, 0};
	color_attachments[0].pNext = nullptr;
	color_attachments[0].imageView = image_view;
	color_attachments[0].imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	color_attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
	color_attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	color_attachments[0].resolveMode = VK_RESOLVE_MODE_NONE;
	color_attachments[0].resolveImageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	color_attachments[0].resolveImageView = VK_NULL_HANDLE;

	VkRenderingAttachmentInfo depth_attachment = {};
	depth_attachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
	depth_attachment.clearValue.color = {0.0f, 0.0f, 0.0f, 1.0f};
	depth_attachment.clearValue.depthStencil = {1.0f, 0};
	depth_attachment.pNext = nullptr;
	depth_attachment.imageView = depth_image_view;
	depth_attachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	depth_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depth_attachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depth_attachment.resolveMode = VK_RESOLVE_MODE_NONE;
	depth_attachment.resolveImageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	depth_attachment.resolveImageView = VK_NULL_HANDLE;

	VkRenderingInfo rendering_info = {};
	rendering_info.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
	rendering_info.pNext = nullptr;
	rendering_info.flags = 0;
	rendering_info.pColorAttachments = color_attachments;
	rendering_info.colorAttachmentCount = 1;
	rendering_info.pDepthAttachment = &depth_attachment;
	rendering_info.layerCount = 1;
	rendering_info.renderArea = {0, 0, init.swapchain.extent.width, init.swapchain.extent.height};

	init.disp.cmdBeginRendering(command_buffer, &rendering_info);
}



int record_command_buffer(Init& init, RenderData& data, uint32_t imageIndex) {

    init.disp.resetCommandBuffer(data.command_buffers[imageIndex], 0);

	auto command_buffer = data.command_buffers[imageIndex];

    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    if (init.disp.beginCommandBuffer(command_buffer, &beginInfo) != VK_SUCCESS) {
        std::cout << "failed to begin recording command buffer\n";
        return -1;
    }

	// transition undefined images
	transition_image_to_color_attachment(init, command_buffer, data.swapchain_images[imageIndex]);
	transition_image_to_depth_attachment(init, command_buffer, data.depth_image.image);

	// render the cube map
	begin_debug_label(init, command_buffer, "cubemap", {0.0, 0.0, 1.0});
	render_cubemap(init, data, data.cube_map, command_buffer, imageIndex);
	end_debug_label(init, command_buffer);

    transition_shadowmap_to_depth_attachment(init, command_buffer, data.shadow_map.image);

    // begin shadow rendering
	begin_debug_label(init, command_buffer, "shadow map", {0.0f, 1.0f, 0.0f});
    draw_shadow(init, data, command_buffer, imageIndex);
	end_debug_label(init, command_buffer);

    transition_shadowmap_to_shader_read(init, command_buffer, data.shadow_map.image);

    // begin main rendering
	// add debug label

    begin_debug_label(init, command_buffer, "main rendering", {1.0f, 0.0f, 0.0f});
	begin_rendering(init, command_buffer, data.swapchain_image_views[imageIndex], data.depth_image.imageView);
    init.disp.cmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, data.graphics_pipeline);

    VkViewport viewport{
	    .x        = 0.0f,
	    .y        = 0.0f,
	    .width    = static_cast<float>(init.swapchain.extent.width),
	    .height   = static_cast<float>(init.swapchain.extent.height),
	    .minDepth = 0.0f,
	    .maxDepth = 1.0f};

    VkRect2D scissor{
	    .offset = {0, 0},
	    .extent = init.swapchain.extent};

    init.disp.cmdSetViewport(command_buffer, 0, 1, &viewport);
	init.disp.cmdSetScissor(command_buffer, 0, 1, &scissor);

    // set the push constants
	const PushConstantBuffer push_constants = {1.0f, true};
	init.disp.cmdPushConstants(command_buffer, data.pipeline_layout, VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PushConstantBuffer), &push_constants);

	//draw_mesh(init, data, command_buffer, data.meshes[0], imageIndex);
    for (const auto& mesh : data.meshes) {
        draw_mesh(init, data, command_buffer, mesh, imageIndex);
    }

	// end rendering
	init.disp.cmdEndRendering(command_buffer);
	end_debug_label(init, command_buffer);

	render_imgui(init, data.command_buffers[imageIndex], data.swapchain_image_views[imageIndex]);

	transition_image_to_present(init, data.command_buffers[imageIndex], data.swapchain_images[imageIndex]);

    if (init.disp.endCommandBuffer(data.command_buffers[imageIndex]) != VK_SUCCESS) {
        spdlog::error("failed to record command buffer");
        return -1;
    }

    return 0;
}

int create_sync_objects(Init& init, RenderData& data) {
    data.available_semaphores.resize(MAX_FRAMES_IN_FLIGHT);
    data.finished_semaphore.resize(MAX_FRAMES_IN_FLIGHT);
    data.in_flight_fences.resize(MAX_FRAMES_IN_FLIGHT);
    data.image_in_flight.resize(init.swapchain.image_count, VK_NULL_HANDLE);

    VkSemaphoreCreateInfo semaphore_info = {};
    semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fence_info = {};
    fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        if (init.disp.createSemaphore(&semaphore_info, nullptr, &data.available_semaphores[i]) != VK_SUCCESS ||
            init.disp.createSemaphore(&semaphore_info, nullptr, &data.finished_semaphore[i]) != VK_SUCCESS ||
            init.disp.createFence(&fence_info, nullptr, &data.in_flight_fences[i]) != VK_SUCCESS) {
            std::cout << "failed to create sync objects\n";
            return -1; // failed to create synchronization objects for a frame
        }
    }
    return 0;
}

int recreate_swapchain(Init& init, RenderData& data) {
    VkResult result = init.disp.deviceWaitIdle();
	if (result != VK_SUCCESS)
	{
		std::cout << "failed to wait for device idle\n";
	}

    init.disp.destroyCommandPool(data.command_pool, nullptr);

    init.swapchain.destroy_image_views(data.swapchain_image_views);

    if (0 != create_swapchain(init)) return -1;
    if (0 != create_command_pool(init, data)) return -1;
    if (0 != create_command_buffers(init, data)) return -1;
    return 0;
}

int draw_frame(Init& init, RenderData& data) {
    init.disp.waitForFences(1, &data.in_flight_fences[data.current_frame], VK_TRUE, UINT64_MAX);

    uint32_t image_index = 0;
    VkResult result = init.disp.acquireNextImageKHR(
            init.swapchain, UINT64_MAX, data.available_semaphores[data.current_frame], VK_NULL_HANDLE, &image_index);

    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        return recreate_swapchain(init, data);
    } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        std::cout << "failed to acquire swapchain image. Error " << result << "\n";
        return -1;
    }

    if (data.image_in_flight[image_index] != VK_NULL_HANDLE) {
        init.disp.waitForFences(1, &data.image_in_flight[image_index], VK_TRUE, UINT64_MAX);
    }
    data.image_in_flight[image_index] = data.in_flight_fences[data.current_frame];

	// update state
	update_uniform_buffer(image_index, init, data);
	update_shadow(init, data);

    // Record the command buffer for this frame
    if (record_command_buffer(init, data, image_index) != 0) {
        return -1;
    }

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore wait_semaphores[] = { data.available_semaphores[data.current_frame] };
    VkPipelineStageFlags wait_stages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = wait_semaphores;
    submitInfo.pWaitDstStageMask = wait_stages;

    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &data.command_buffers[image_index];

    VkSemaphore signal_semaphores[] = { data.finished_semaphore[data.current_frame] };
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signal_semaphores;

    init.disp.resetFences(1, &data.in_flight_fences[data.current_frame]);

    if (init.disp.queueSubmit(init.graphics_queue, 1, &submitInfo, data.in_flight_fences[data.current_frame]) != VK_SUCCESS) {
        std::cout << "failed to submit draw command buffer\n";
        return -1; //"failed to submit draw command buffer
    }

    VkPresentInfoKHR present_info = {};
    present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    present_info.waitSemaphoreCount = 1;
    present_info.pWaitSemaphores = signal_semaphores;

    VkSwapchainKHR swapChains[] = { init.swapchain };
    present_info.swapchainCount = 1;
    present_info.pSwapchains = swapChains;

    present_info.pImageIndices = &image_index;

    result = init.disp.queuePresentKHR(init.graphics_queue, &present_info);
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
        return recreate_swapchain(init, data);
    } else if (result != VK_SUCCESS) {
        std::cout << "failed to present swapchain image\n";
        return -1;
    }

    data.current_frame = (data.current_frame + 1) % MAX_FRAMES_IN_FLIGHT;
    return 0;
}

void cleanup(Init& init, RenderData& data) {

    init.disp.deviceWaitIdle();

    // Clean up depth resources
    cleanup_image(init, data.depth_image);

    // Keep this loop for uniform buffer cleanup
    for (size_t i = 0; i < init.swapchain.image_count; i++) {
        vmaDestroyBuffer(init.allocator, data.uniform_buffers[i].buffer, data.uniform_buffers[i].allocation);
    }

    // Cleanup ImGui
    destroy_imgui();

    init.disp.destroyDescriptorSetLayout(data.descriptor_set_layout, nullptr);

    init.disp.destroyDescriptorPool(data.descriptor_pool, nullptr);

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        init.disp.destroySemaphore(data.finished_semaphore[i], nullptr);
        init.disp.destroySemaphore(data.available_semaphores[i], nullptr);
        init.disp.destroyFence(data.in_flight_fences[i], nullptr);
    }

    init.disp.freeCommandBuffers(data.command_pool, data.command_buffers.size(), data.command_buffers.data());

    init.disp.destroyCommandPool(data.command_pool, nullptr);

    init.disp.destroyPipeline(data.graphics_pipeline, nullptr);
    init.disp.destroyPipelineLayout(data.pipeline_layout, nullptr);

    init.swapchain.destroy_image_views(data.swapchain_image_views);

    vkb::destroy_swapchain(init.swapchain);

    vmaDestroyAllocator(init.allocator);

    init.disp.destroyCommandPool(init.command_pool, nullptr);

    vkb::destroy_device(init.device);
    vkb::destroy_surface(init.instance, init.surface);
    vkb::destroy_instance(init.instance);
    destroy_window_glfw(init.window);
}



int create_descriptor_pool(Init& init, RenderData& data) {
    VkDescriptorPoolSize pool_sizes[] =
            {
                    {VK_DESCRIPTOR_TYPE_SAMPLER, 1000},
                    {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000},
                    {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000},
                    {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000},
                    {VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000},
                    {VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000},
                    {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000},
                    {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000},
                    {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000},
                    {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000},
                    {VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000}
            };

    VkDescriptorPoolCreateInfo pool_info = {};
    pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    pool_info.maxSets = 10;
    pool_info.poolSizeCount = (uint32_t) IM_ARRAYSIZE(pool_sizes);
    pool_info.pPoolSizes = pool_sizes;


    if (init.disp.createDescriptorPool(&pool_info, nullptr, &data.descriptor_pool) != VK_SUCCESS) {
        std::cout << "failed to create descriptor pool\n";
        return -1; // failed to create descriptor pool
    }
    return 0; 
}

void processInput(GLFWwindow* window, float deltaTime, Camera* camera) {
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera->process_keyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera->process_keyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera->process_keyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera->process_keyboard(RIGHT, deltaTime);
}

int create_descriptor_set_layout(Init& init, RenderData& renderData) {

    VkDescriptorSetLayoutBinding ubo_layout_binding = {
	    .binding            = 0,
	    .descriptorType     = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
	    .descriptorCount    = 1,
	    .stageFlags         = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
	    .pImmutableSamplers = nullptr,
	};

    // add the material binding
	VkDescriptorSetLayoutBinding material_layout_binding = {
	    .binding            = 1,
	    .descriptorType     = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
	    .descriptorCount    = 1,
	    .stageFlags         = VK_SHADER_STAGE_FRAGMENT_BIT,
	    .pImmutableSamplers = nullptr,
	};

    // add the sampler binding
    VkDescriptorSetLayoutBinding sampler_layout_binding = {
            .binding = 2,
            .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .descriptorCount = 1,
            .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
            .pImmutableSamplers = nullptr,
    };

    VkDescriptorSetLayoutBinding normal_sampler_layout_binding = {
	    .binding            = 3,
	    .descriptorType     = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
	    .descriptorCount    = 1,
	    .stageFlags         = VK_SHADER_STAGE_FRAGMENT_BIT,
	    .pImmutableSamplers = nullptr,
	};

    // add cube map sampler binding
    VkDescriptorSetLayoutBinding cubemap_sampler_layout_binding = {
            .binding = 4,
            .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .descriptorCount = 1,
            .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
            .pImmutableSamplers = nullptr,
    };

	// add shadow map sampler binding
	VkDescriptorSetLayoutBinding shadowmap_sampler_layout_binding = {
		.binding = 5,
		.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
		.descriptorCount = 1,
		.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
		.pImmutableSamplers = nullptr,
	};

    std::array<VkDescriptorSetLayoutBinding, 6> bindings = {
            ubo_layout_binding,
            material_layout_binding,
            sampler_layout_binding,
	        normal_sampler_layout_binding,
            cubemap_sampler_layout_binding,
			shadowmap_sampler_layout_binding,
    };

    VkDescriptorSetLayoutCreateInfo layout_info = {};
    layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layout_info.bindingCount = static_cast<uint32_t>(bindings.size());
    layout_info.pBindings = bindings.data();

    if (init.disp.createDescriptorSetLayout(&layout_info, nullptr, &renderData.descriptor_set_layout) != VK_SUCCESS) {
        std::cout << "failed to create descriptor set layout\n";
        return -1;
    }
    return 0;
}

int create_descriptor_sets(Init& init, RenderData& renderData) {
    std::vector<VkDescriptorSetLayout> layouts(init.swapchain.image_count, renderData.descriptor_set_layout);

    VkDescriptorSetAllocateInfo alloc_info = {};
    alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    alloc_info.descriptorPool = renderData.descriptor_pool;
    alloc_info.descriptorSetCount = static_cast<uint32_t>(init.swapchain.image_count);
    alloc_info.pSetLayouts = layouts.data();

    renderData.descriptor_sets.resize(init.swapchain.image_count);
    if (init.disp.allocateDescriptorSets(&alloc_info, renderData.descriptor_sets.data()) != VK_SUCCESS) {
        std::cout << "failed to allocate descriptor sets\n";
        return -1;
    }

    for (size_t i = 0; i < init.swapchain.image_count; i++) {

        VkDescriptorBufferInfo buffer_info = {
		    .buffer = renderData.uniform_buffers[i].buffer,
		    .offset = 0,
		    .range  = sizeof(UniformBufferObject),
		};

        VkDescriptorBufferInfo material_buffer_info = {
		    .buffer = renderData.material_buffer.buffer,
		    .offset = 0,
		    .range  = sizeof(MaterialBufferObject),
		};

        VkDescriptorImageInfo image_info = {
            .sampler = renderData.texture.sampler,
            .imageView = renderData.texture.view,
            .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        };

        VkDescriptorImageInfo normal_image_info = {
		    .sampler     = renderData.materials[0].normal_map.sampler,
		    .imageView   = renderData.materials[0].normal_map.view,
		    .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
		};

        VkDescriptorImageInfo cubemap_image_info = {
            .sampler = renderData.cube_map_texture.sampler,
            .imageView = renderData.cube_map_texture.view,
            .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        };

		VkDescriptorImageInfo shadowmap_image_info = {
		    .sampler     = renderData.shadow_map.sampler,
		    .imageView   = renderData.shadow_map.image_view,
		    .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};

        std::array<VkWriteDescriptorSet, 6> descriptor_writes = {};

        descriptor_writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptor_writes[0].dstSet = renderData.descriptor_sets[i];
        descriptor_writes[0].dstBinding = 0;
        descriptor_writes[0].dstArrayElement = 0;
        descriptor_writes[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptor_writes[0].descriptorCount = 1;
        descriptor_writes[0].pBufferInfo = &buffer_info;

        descriptor_writes[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptor_writes[1].dstSet = renderData.descriptor_sets[i];
		descriptor_writes[1].dstBinding = 1;
		descriptor_writes[1].dstArrayElement = 0;
		descriptor_writes[1].descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptor_writes[1].descriptorCount = 1;
		descriptor_writes[1].pBufferInfo     = &material_buffer_info;

        descriptor_writes[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptor_writes[2].dstSet = renderData.descriptor_sets[i];
        descriptor_writes[2].dstBinding = 2;
        descriptor_writes[2].dstArrayElement = 0;
        descriptor_writes[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptor_writes[2].descriptorCount = 1;
        descriptor_writes[2].pImageInfo = &image_info;

        descriptor_writes[3].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptor_writes[3].dstSet = renderData.descriptor_sets[i];
		descriptor_writes[3].dstBinding = 3;
		descriptor_writes[3].dstArrayElement = 0;
		descriptor_writes[3].descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptor_writes[3].descriptorCount = 1;
		descriptor_writes[3].pImageInfo      = &normal_image_info;

        descriptor_writes[4].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptor_writes[4].dstSet = renderData.descriptor_sets[i];
        descriptor_writes[4].dstBinding = 4;
        descriptor_writes[4].dstArrayElement = 0;
        descriptor_writes[4].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptor_writes[4].descriptorCount = 1;
        descriptor_writes[4].pImageInfo = &cubemap_image_info;

		descriptor_writes[5].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptor_writes[5].dstSet = renderData.descriptor_sets[i];
		descriptor_writes[5].dstBinding = 5;
		descriptor_writes[5].dstArrayElement = 0;
		descriptor_writes[5].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptor_writes[5].descriptorCount = 1;
		descriptor_writes[5].pImageInfo = &shadowmap_image_info;

        init.disp.updateDescriptorSets(descriptor_writes.size(), descriptor_writes.data(), 0, nullptr);
    }
    return 0;
}

int create_uniform_buffers(Init& init, RenderData& renderData) {
    renderData.uniform_buffers.resize(init.swapchain.image_count);
    for (size_t i = 0; i < init.swapchain.image_count; i++) {

    	BufferAllocation buffer = create_buffer(
    		init,
    		sizeof(UniformBufferObject), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
    		VMA_MEMORY_USAGE_CPU_TO_GPU);

    	renderData.uniform_buffers[i] = buffer;

    }

    return 0;
}



void configure_mouse_input(const Init& init, RenderData& render_data) {
	glfwSetWindowUserPointer(init.window, &render_data);
	glfwSetCursorPosCallback(init.window, mouse_callback);
	glfwSetInputMode(init.window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}

void setup_swapchain_images(Init& init, RenderData& render_data)
{
	render_data.swapchain_images = init.swapchain.get_images().value();
	render_data.swapchain_image_views = init.swapchain.get_image_views().value();
}

void create_scene_data_buffers(const Init& init, RenderData& render_data, const std::vector<MeshData>& mesh_data)
{

    // Create staging buffer
	uint64_t staging_buffer_size      = 1000000;
    spdlog::info("Creating staging buffers of size {}", staging_buffer_size);
	render_data.vertex_staging_buffer = create_staging_buffer(init, staging_buffer_size);
	render_data.index_staging_buffer  = create_staging_buffer(init, staging_buffer_size);

    uint32_t total_vertex_count = 0;
    uint32_t total_index_count = 0;

    for (const auto& mesh : mesh_data) {
        total_vertex_count += mesh.vertices.size();
        total_index_count += mesh.indices.size();
    }

    // create the vertex buffer
	render_data.vertex_buffers[0] = create_buffer(
	    init,
	    total_vertex_count * sizeof(Vertex),
	    VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
	    VMA_MEMORY_USAGE_GPU_ONLY); 

	// create the index buffer
	render_data.index_buffers[0] = create_buffer(
	    init,
	    total_index_count * sizeof(uint32_t),
	    VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
	    VMA_MEMORY_USAGE_GPU_ONLY);

	// Copy data to staging buffer
	uint32_t vertex_offset = 0;
	uint32_t index_offset  = 0;

	for (const auto &mesh : mesh_data)
	{
		uint32_t vertex_size = mesh.vertices.size() * sizeof(Vertex);
		uint32_t index_size  = mesh.indices.size() * sizeof(uint32_t);

        // copy the vertex data over to the staging buffer directly
		copy_buffer_data(
		    init,
		    render_data.vertex_staging_buffer,
		    vertex_offset * sizeof(Vertex),
		    vertex_size,
		    mesh.vertices.data());

		// increment the indices by the index offset copy indices first
		std::vector<uint32_t> adjusted_indices = mesh.indices;
		for (auto &index : adjusted_indices)
		{
			index += vertex_offset;
		}

        // copy the index data over to the staging buffer directly
		copy_buffer_data(
		    init,
		    render_data.index_staging_buffer,
		    index_offset * sizeof(uint32_t),
		    index_size,
		    adjusted_indices.data());

		// Create the Mesh struct and add it to the vector
		Mesh new_mesh;
		new_mesh.vertex_buffer_handle = 0; // Assuming we're using the first vertex buffer
		new_mesh.index_buffer_handle = 0;  // Assuming we're using the first index buffer
		new_mesh.submeshes.push_back({
			.start_index = index_offset,
			.index_count = static_cast<uint32_t>(mesh.indices.size()),
		});
		new_mesh.material_index = 0; // Assign a unique material index

		render_data.meshes.push_back(new_mesh);

		// Update the index_offset for the next mesh
		index_offset += mesh.indices.size();
        vertex_offset += mesh.vertices.size();

	}

    VkCommandBuffer command_buffer = begin_single_time_commands(init);

    // Copy vertex staging buffer to vertex buffer
    VkBufferCopy vertex_copy_region{};
    vertex_copy_region.srcOffset = 0;
    vertex_copy_region.dstOffset = 0;
    vertex_copy_region.size = render_data.vertex_staging_buffer.size;
    init.disp.cmdCopyBuffer(command_buffer, render_data.vertex_staging_buffer.buffer, render_data.vertex_buffers[0].buffer, 1, &vertex_copy_region);

    // Copy index staging buffer to index buffer
    VkBufferCopy index_copy_region{};
    index_copy_region.srcOffset = 0;
    index_copy_region.dstOffset = 0;
    index_copy_region.size = render_data.index_staging_buffer.size;
    init.disp.cmdCopyBuffer(command_buffer, render_data.index_staging_buffer.buffer, render_data.index_buffers[0].buffer, 1, &index_copy_region);

    // Add memory barriers to ensure the copy is complete before using the buffers
    VkBufferMemoryBarrier vertex_barrier{};
    vertex_barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
    vertex_barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    vertex_barrier.dstAccessMask = VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT;
    vertex_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    vertex_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    vertex_barrier.buffer = render_data.vertex_buffers[0].buffer;
    vertex_barrier.offset = 0;
    vertex_barrier.size = VK_WHOLE_SIZE;

    VkBufferMemoryBarrier index_barrier{};
    index_barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
    index_barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    index_barrier.dstAccessMask = VK_ACCESS_INDEX_READ_BIT;
    index_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    index_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    index_barrier.buffer = render_data.index_buffers[0].buffer;
    index_barrier.offset = 0;
    index_barrier.size = VK_WHOLE_SIZE;

    VkBufferMemoryBarrier barriers[] = {vertex_barrier, index_barrier};

    init.disp.cmdPipelineBarrier(
        command_buffer,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        VK_PIPELINE_STAGE_VERTEX_INPUT_BIT,
        0,
        0, nullptr,
        2, barriers,
        0, nullptr
    );


    end_single_time_commands(init, command_buffer);
}

void setup_materials(const Init& init, RenderData& render_data)
{
	// Create the material buffer
	render_data.material_buffer = create_buffer(
	    init,
	    sizeof(MaterialBufferObject) * render_data.materials.size(),
	    VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
	    VMA_MEMORY_USAGE_CPU_TO_GPU);

    
}

void setup_scene(const Init& init, RenderData& render_data, ImageLoader* image_loader)
{
    spdlog::info("Setting up scene");
	// set up camera 
	render_data.camera = new Camera();
	render_data.camera->position = glm::vec3(-2.2f, 1.66f, 1.7f);
	render_data.camera->look_at(glm::vec3(0.0f));

	// load the mesh
	MeshData truck_model = create_from_obj("assets/meshes/cube.obj");
	MeshData plane_model = create_plane(10, 10);

    std::vector<MeshData> mesh_data = {
        truck_model, plane_model
    };

    create_scene_data_buffers(init, render_data, mesh_data);

    // create materials
	Material truck_material{};
    truck_material.albedo_map = render_data.texture;
    truck_material.normal_map = image_loader->load_texture("assets/textures/cobblestone_n.ktx2", true);

    truck_material.roughness = 0.5f;
    truck_material.metallic = 0.5f;

    render_data.materials.push_back(truck_material);

    // assign materials to meshes
    render_data.meshes[0].material_index = 0;
    render_data.meshes[1].material_index = 1;

    setup_materials(init, render_data);

}

int main() {
    Init init;
    RenderData render_data;

    if (0 != device_initialization(init)) return -1;
    if (0 != create_swapchain(init)) return -1;
    if (0 != create_descriptor_set_layout(init, render_data)) return -1;
	setup_swapchain_images(init, render_data);

	render_data.depth_image = create_depth_image(init, VK_FORMAT_D16_UNORM);
    if (0 != create_graphics_pipeline(init, render_data)) return -1;
    if (0 != create_command_pool(init, render_data)) return -1;
    if (0 != create_command_buffers(init, render_data)) return -1;
    if (0 != create_sync_objects(init, render_data)) return -1;
    if (0 != create_descriptor_pool(init, render_data)) return -1;
    if (0 != create_imgui(init, render_data)) return -1;
    if (0 != create_uniform_buffers(init, render_data)) return -1;

	init_shadow_pipeline(init, render_data);
	init_shadow_map(init, render_data);


    ImageLoader* imageLoader = new ImageLoader(init);
    render_data.texture = imageLoader->load_texture("assets/textures/cobblestone_d.ktx2");
    render_data.cube_map_texture = imageLoader->load_cubemap("assets/textures/clouds.ktx2");

	render_data.cube_map = create_cubemap(init, render_data.descriptor_set_layout);

    setup_scene(init, render_data, imageLoader);

    if (0 != create_descriptor_sets(init, render_data)) return -1;

    auto lastTime = std::chrono::high_resolution_clock::now();
    float deltaTime = 0.0f;

	configure_mouse_input(init, render_data);

	const auto cmdBuffer = begin_single_time_commands(init);
	transition_shadowmap_initial(init, cmdBuffer, render_data.shadow_map.image);
	end_single_time_commands(init, cmdBuffer);

    while (!glfwWindowShouldClose(init.window)) {

        auto currentTime = std::chrono::high_resolution_clock::now();
        deltaTime = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - lastTime).count();
        lastTime = currentTime;

        glfwPollEvents();

        // check if escape key is pressed
        if (glfwGetKey(init.window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            glfwSetWindowShouldClose(init.window, true);
        }

        processInput(init.window, deltaTime, render_data.camera);

		create_new_imgui_frame(render_data);
        int res = draw_frame(init, render_data);
        if (res != 0) {
		    std::cout << "failed to draw frame \n";
            return -1;
	    }

    }
    init.disp.deviceWaitIdle();

    cleanup_cubemap(init, render_data.cube_map);
    delete imageLoader;

	cleanup_shadow_map(init, render_data);

    cleanup(init, render_data);
    return 0;
}