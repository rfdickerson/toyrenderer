#include "common.hpp"
#include "utils.hpp"

#include "../extern/imgui/imgui.h"
#include "../extern/imgui/imgui_impl_glfw.h"
#include "../extern/imgui/imgui_impl_vulkan.h"

#include "camera.hpp"
#include "image_loader.hpp"
#include "cube_map.hpp"
#include "mesh.hpp"
#include "debug_utils.hpp"
#include "shadow.hpp"
#include "obj_loader.hpp"

using namespace obsidian;

const int WIDTH = 1280;
const int HEIGHT = 720;

const int MAX_FRAMES_IN_FLIGHT = 2;

struct PushConstantBuffer {
	float scale;
	bool useTexture;
};


void mouse_callback(GLFWwindow *window, double xpos, double ypos) {
    auto data = static_cast<RenderData *>(glfwGetWindowUserPointer(window));

    if (data->firstMouse) {
        data->lastX = xpos;
        data->lastY = ypos;
        data->firstMouse = false;
    }

    float xoffset = xpos - data->lastX;
    float yoffset = data->lastY - ypos; // reversed since y-coordinates go from bottom to top
    data->lastX = xpos;
    data->lastY = ypos;

	data->camera.process_mouse_movement(xoffset, yoffset);
}


int create_depth_resources(Init& init, RenderData& data) {
    //VkFormat depthFormat = findDepthFormat(init);
	const auto depthFormat = VK_FORMAT_D24_UNORM_S8_UINT;

    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = init.swapchain.extent.width;
    imageInfo.extent.height = init.swapchain.extent.height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = depthFormat;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VmaAllocationCreateInfo allocInfo = {};
    allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

    if (vmaCreateImage(init.allocator, &imageInfo, &allocInfo, &data.depth_image.image, &data.depth_image.allocation, nullptr) != VK_SUCCESS) {
        throw std::runtime_error("failed to create depth image!");
    }

    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = data.depth_image.image;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = depthFormat;
    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    if (init.disp.createImageView(&viewInfo, nullptr, &data.depth_image_view) != VK_SUCCESS) {
        throw std::runtime_error("failed to create depth image view!");
    }

    return 0;
}

void copy_buffer(Init& init, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = init.command_pool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    if (init.disp.allocateCommandBuffers(&allocInfo, &commandBuffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate command buffer");
    }

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    if (init.disp.beginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
        throw std::runtime_error("failed to begin command buffer");
    }

    VkBufferCopy copyRegion{};
    copyRegion.size = size;
    init.disp.cmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

    if (init.disp.endCommandBuffer(commandBuffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to end command buffer");
    }

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    VkFence fence;

    if (init.disp.createFence(&fenceInfo, nullptr, &fence) != VK_SUCCESS) {
        throw std::runtime_error("failed to create fence");
    }

    if (init.disp.queueSubmit(init.graphics_queue, 1, &submitInfo, fence) != VK_SUCCESS) {
        throw std::runtime_error("failed to submit copy command buffer");
    }

    if (init.disp.waitForFences(1, &fence, VK_TRUE, UINT64_MAX) != VK_SUCCESS) {
        throw std::runtime_error("failed to wait for fence");
    }

    init.disp.destroyFence(fence, nullptr);

    init.disp.freeCommandBuffers(init.command_pool, 1, &commandBuffer);
}

void copy_buffer_data(Init& init, BufferAllocation buffer, VkDeviceSize size, void* data) {
	void *mapped_data;
	vmaMapMemory(init.allocator, buffer.allocation, &mapped_data);
	memcpy(mapped_data, data, size);
	vmaUnmapMemory(init.allocator, buffer.allocation);
}

void update_uniform_buffer(uint32_t current, Init &init, RenderData& renderData) {

	UniformBufferObject ubo {
		.model = glm::mat4(1.0f),
		.view = renderData.camera.getViewMatrix(),
		.proj = renderData.camera.getProjectionMatrix(),
	    .lightSpaceMatrix = renderData.shadow_map.light_space_matrix,
	    .lightDirection = renderData.shadow_map.light_direction
	};

	copy_buffer_data(init, renderData.uniform_buffers[current], sizeof(ubo), &ubo);
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

int create_render_pass(Init& init, RenderData& data) {
    VkAttachmentDescription color_attachment = {};
    color_attachment.format = init.swapchain.image_format;
    color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
    color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
    color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    color_attachment.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentDescription depthAttachment = {};
    depthAttachment.format = VK_FORMAT_D24_UNORM_S8_UINT;
    depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference color_attachment_ref = {};
    color_attachment_ref.attachment = 0;
    color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depthAttachmentRef = {};
    depthAttachmentRef.attachment = 1;
    depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &color_attachment_ref;
    subpass.pDepthStencilAttachment = &depthAttachmentRef;

    VkSubpassDependency dependency = {};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    std::array<VkAttachmentDescription, 2> attachments = {color_attachment, depthAttachment};
    VkRenderPassCreateInfo render_pass_info = {};
    render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    render_pass_info.attachmentCount = static_cast<uint32_t>(attachments.size());
    render_pass_info.pAttachments = attachments.data();
    render_pass_info.subpassCount = 1;
    render_pass_info.pSubpasses = &subpass;
    render_pass_info.dependencyCount = 1;
    render_pass_info.pDependencies = &dependency;

    if (init.disp.createRenderPass(&render_pass_info, nullptr, &data.render_pass) != VK_SUCCESS) {
        std::cout << "failed to create render pass\n";
        return -1; // failed to create render pass!
    }
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

    std::array<VkVertexInputAttributeDescription, 4> attributeDescriptions{};

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

//	VkPushConstantRange pushConstantRange = {};
//	pushConstantRange.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
//	pushConstantRange.offset = 0;
//	pushConstantRange.size = sizeof(float);
//

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
	renderingCreateInfo.depthAttachmentFormat = VK_FORMAT_D24_UNORM_S8_UINT;

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
        std::cout << "failed to create pipline\n";
        return -1; // failed to create graphics pipeline
    }

    init.disp.destroyShaderModule(frag_module, nullptr);
    init.disp.destroyShaderModule(vert_module, nullptr);
    return 0;
}

int create_framebuffers(Init& init, RenderData& data) {
    data.swapchain_images = init.swapchain.get_images().value();
    data.swapchain_image_views = init.swapchain.get_image_views().value();

    data.framebuffers.resize(data.swapchain_image_views.size());

    for (size_t i = 0; i < data.swapchain_image_views.size(); i++) {
        std::array<VkImageView, 2> attachments = {
                data.swapchain_image_views[i],
                data.depth_image_view
        };

        VkFramebufferCreateInfo framebuffer_info = {};
        framebuffer_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebuffer_info.renderPass = data.render_pass;
        framebuffer_info.attachmentCount = static_cast<uint32_t>(attachments.size());
        framebuffer_info.pAttachments = attachments.data();
        framebuffer_info.width = init.swapchain.extent.width;
        framebuffer_info.height = init.swapchain.extent.height;
        framebuffer_info.layers = 1;

        if (init.disp.createFramebuffer(&framebuffer_info, nullptr, &data.framebuffers[i]) != VK_SUCCESS) {
            return -1; // failed to create framebuffer
        }
    }
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

int render_cubemap(Init& init, RenderData& data, uint32_t imageIndex) {

    VkCommandBuffer commandBuffer = data.command_buffers[imageIndex];

    std::array<VkClearValue, 2> clearValues = {};
    clearValues[0].color = {{0.0f, 0.0f, 0.0f, 1.0f}};
    clearValues[1].depthStencil = {1.0f, 0};

    VkRenderPassBeginInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = data.cube_map->render_pass;
    renderPassInfo.framebuffer = data.framebuffers[imageIndex];
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = init.swapchain.extent;
    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassInfo.pClearValues = clearValues.data();


    init.disp.cmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    VkViewport viewport = {};
    viewport.width = static_cast<float>(init.swapchain.extent.width);
    viewport.height = static_cast<float>(init.swapchain.extent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor = {};
    scissor.extent = init.swapchain.extent;


    init.disp.cmdSetViewport(commandBuffer, 0, 1, &viewport);
    init.disp.cmdSetScissor(commandBuffer, 0, 1, &scissor);
    init.disp.cmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, data.cube_map->pipeline);


    init.disp.cmdBindDescriptorSets(commandBuffer,
                                    VK_PIPELINE_BIND_POINT_GRAPHICS,
                                    data.cube_map->pipeline_layout, 0, 1,
                                    &data.descriptor_sets[imageIndex], 0, nullptr);

    init.disp.cmdDraw(commandBuffer, 36, 1, 0, 0);
    init.disp.cmdEndRenderPass(commandBuffer);
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

	begin_debug_label(init, data.command_buffers[imageIndex], "Cube Map Rendering", {1.0f, 0.0f, 0.0f});
	data.cube_map->render(init, data, data.command_buffers[imageIndex], imageIndex);
	init.disp.cmdEndDebugUtilsLabelEXT(data.command_buffers[imageIndex]);

	// transition shadow map image
	transition_shadowmap_to_depth_attachment(init, command_buffer, data.shadow_map.image);

	// shadow map rendering
	begin_debug_label(init, data.command_buffers[imageIndex], "Shadow Map Rendering", {0.0f, 1.0f, 0.0f});
	draw_shadow(init, data, command_buffer, imageIndex);
	init.disp.cmdEndDebugUtilsLabelEXT(data.command_buffers[imageIndex]);

	// transition shadow map image
	transition_shadowmap_to_shader_read(init, command_buffer, data.shadow_map.image);

	begin_debug_label(init, data.command_buffers[imageIndex], "Main Rendering", {1.0f, 1.0f, 0.0f});
	begin_rendering(init, data.command_buffers[imageIndex], data.swapchain_image_views[imageIndex], data.depth_image_view);

	// set scissor and viewport
	VkViewport viewport = {};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = (float)init.swapchain.extent.width;
	viewport.height = (float)init.swapchain.extent.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	VkRect2D scissor = {};
	scissor.offset = {0, 0};
	scissor.extent = init.swapchain.extent;

	init.disp.cmdSetViewport(data.command_buffers[imageIndex], 0, 1, &viewport);
	init.disp.cmdSetScissor(data.command_buffers[imageIndex], 0, 1, &scissor);

	init.disp.cmdBindPipeline(data.command_buffers[imageIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, data.graphics_pipeline);
	// bind descriptor sets
	init.disp.cmdBindDescriptorSets(data.command_buffers[imageIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, data.pipeline_layout, 0, 1, &data.descriptor_sets[imageIndex], 0, nullptr);

	PushConstantBuffer push_constant = {};
	push_constant.scale = 2.0f;
	push_constant.useTexture = true;

	vkCmdPushConstants(data.command_buffers[imageIndex], data.pipeline_layout, VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PushConstantBuffer), &push_constant);

	// draw the bunny
	data.bunny_mesh->draw(init, data.command_buffers[imageIndex]);

	push_constant.scale = 20.0f;
	push_constant.useTexture = false;
	vkCmdPushConstants(data.command_buffers[imageIndex], data.pipeline_layout, VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PushConstantBuffer), &push_constant);

	data.plane_mesh->draw(init, data.command_buffers[imageIndex]);

	init.disp.cmdEndRendering(data.command_buffers[imageIndex]);
	end_debug_label(init, data.command_buffers[imageIndex]);

	render_imgui(init, data.command_buffers[imageIndex], data.swapchain_image_views[imageIndex]);

	transition_image_to_present(init, data.command_buffers[imageIndex], data.swapchain_images[imageIndex]);

    if (init.disp.endCommandBuffer(data.command_buffers[imageIndex]) != VK_SUCCESS) {
        std::cout << "failed to record command buffer\n";
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
    init.disp.deviceWaitIdle();

    init.disp.destroyCommandPool(data.command_pool, nullptr);

    for (auto framebuffer : data.framebuffers) {
        init.disp.destroyFramebuffer(framebuffer, nullptr);
    }

    init.swapchain.destroy_image_views(data.swapchain_image_views);

    if (0 != create_swapchain(init)) return -1;
    if (0 != create_framebuffers(init, data)) return -1;
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
    init.disp.destroyImageView(data.depth_image_view, nullptr);
    vmaDestroyImage(init.allocator, data.depth_image.image, data.depth_image.allocation);

    // Keep this loop for uniform buffer cleanup
    for (size_t i = 0; i < init.swapchain.image_count; i++) {
        vmaDestroyBuffer(init.allocator, data.uniform_buffers[i].buffer, data.uniform_buffers[i].allocation);
    }

    // Cleanup ImGui
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    init.disp.destroyDescriptorSetLayout(data.descriptor_set_layout, nullptr);

    init.disp.destroyDescriptorPool(data.descriptor_pool, nullptr);

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        init.disp.destroySemaphore(data.finished_semaphore[i], nullptr);
        init.disp.destroySemaphore(data.available_semaphores[i], nullptr);
        init.disp.destroyFence(data.in_flight_fences[i], nullptr);
    }

    init.disp.freeCommandBuffers(data.command_pool, data.command_buffers.size(), data.command_buffers.data());

    init.disp.destroyCommandPool(data.command_pool, nullptr);

    for (auto framebuffer : data.framebuffers) {
        init.disp.destroyFramebuffer(framebuffer, nullptr);
    }

    init.disp.destroyPipeline(data.graphics_pipeline, nullptr);
    init.disp.destroyPipelineLayout(data.pipeline_layout, nullptr);
    init.disp.destroyRenderPass(data.render_pass, nullptr);

    init.swapchain.destroy_image_views(data.swapchain_image_views);

    vkb::destroy_swapchain(init.swapchain);

    vmaDestroyAllocator(init.allocator);

    init.disp.destroyCommandPool(init.command_pool, nullptr);

    vkb::destroy_device(init.device);
    vkb::destroy_surface(init.instance, init.surface);
    vkb::destroy_instance(init.instance);
    destroy_window_glfw(init.window);
}

int create_imgui(Init& init, RenderData& data) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();

	VkPipelineRenderingCreateInfo pipeline_rendering_create_info = {};
	pipeline_rendering_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
	pipeline_rendering_create_info.depthAttachmentFormat = VK_FORMAT_D24_UNORM_S8_UINT;
	pipeline_rendering_create_info.pNext = nullptr;
	pipeline_rendering_create_info.pColorAttachmentFormats = &init.swapchain.image_format;
	pipeline_rendering_create_info.colorAttachmentCount = 1;

    ImGui_ImplGlfw_InitForVulkan(init.window, true);
    ImGui_ImplVulkan_InitInfo init_info = {};
    init_info.Instance = init.instance.instance;
    init_info.PhysicalDevice = init.physical_device.physical_device;
    init_info.Device = init.device.device;
    init_info.QueueFamily = init.device.get_queue_index(vkb::QueueType::graphics).value();
    init_info.Queue = init.graphics_queue;
    init_info.PipelineCache = VK_NULL_HANDLE;
    init_info.DescriptorPool = data.descriptor_pool;
    init_info.Allocator = nullptr;
    init_info.MinImageCount = init.swapchain.image_count;
    init_info.ImageCount = init.swapchain.image_count;
    init_info.CheckVkResultFn = nullptr;
    init_info.RenderPass = nullptr;
	init_info.UseDynamicRendering = VK_TRUE;
	init_info.PipelineRenderingCreateInfo = pipeline_rendering_create_info;
    ImGui_ImplVulkan_Init(&init_info);

    ImGui_ImplVulkan_CreateFontsTexture();

    return 0;
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



void processInput(GLFWwindow* window, float deltaTime, Camera& camera) {
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera.process_keyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.process_keyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.process_keyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.process_keyboard(RIGHT, deltaTime);
}

int create_descriptor_set_layout(Init& init, RenderData& renderData) {
    VkDescriptorSetLayoutBinding ubo_layout_binding = {};
    ubo_layout_binding.binding = 0;
    ubo_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    ubo_layout_binding.descriptorCount = 1;
    ubo_layout_binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    ubo_layout_binding.pImmutableSamplers = nullptr;

    // add the sampler binding
    VkDescriptorSetLayoutBinding sampler_layout_binding = {
            .binding = 1,
            .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .descriptorCount = 1,
            .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
            .pImmutableSamplers = nullptr,
    };

    // add cube map sampler binding
    VkDescriptorSetLayoutBinding cubemap_sampler_layout_binding = {
            .binding = 2,
            .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .descriptorCount = 1,
            .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
            .pImmutableSamplers = nullptr,
    };

	// add shadow map sampler binding
	VkDescriptorSetLayoutBinding shadowmap_sampler_layout_binding = {
		.binding = 3,
		.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
		.descriptorCount = 1,
		.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
		.pImmutableSamplers = nullptr,
	};

    std::array<VkDescriptorSetLayoutBinding, 4> bindings = {
            ubo_layout_binding,
            sampler_layout_binding,
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

int  create_descriptor_sets(Init& init, RenderData& renderData) {
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
        VkDescriptorBufferInfo buffer_info = {};
        buffer_info.buffer = renderData.uniform_buffers[i].buffer;
        buffer_info.offset = 0;
        buffer_info.range = sizeof(UniformBufferObject);

        VkDescriptorImageInfo image_info = {
            .sampler = renderData.texture.sampler,
            .imageView = renderData.texture.view,
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

        std::array<VkWriteDescriptorSet, 4> descriptor_writes = {};

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
        descriptor_writes[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptor_writes[1].descriptorCount = 1;
        descriptor_writes[1].pImageInfo = &image_info;

        descriptor_writes[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptor_writes[2].dstSet = renderData.descriptor_sets[i];
        descriptor_writes[2].dstBinding = 2;
        descriptor_writes[2].dstArrayElement = 0;
        descriptor_writes[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptor_writes[2].descriptorCount = 1;
        descriptor_writes[2].pImageInfo = &cubemap_image_info;

		descriptor_writes[3].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptor_writes[3].dstSet = renderData.descriptor_sets[i];
		descriptor_writes[3].dstBinding = 3;
		descriptor_writes[3].dstArrayElement = 0;
		descriptor_writes[3].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptor_writes[3].descriptorCount = 1;
		descriptor_writes[3].pImageInfo = &shadowmap_image_info;

        init.disp.updateDescriptorSets(descriptor_writes.size(), descriptor_writes.data(), 0, nullptr);
    }
    return 0;
}

int create_uniform_buffers(Init& init, RenderData& renderData) {
    renderData.uniform_buffers.resize(init.swapchain.image_count);
    for (size_t i = 0; i < init.swapchain.image_count; i++) {
        create_buffer(init,
                      sizeof(UniformBufferObject),
                      VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                      VMA_MEMORY_USAGE_CPU_TO_GPU,
                      renderData.uniform_buffers[i]);
    }

    return 0;
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
	ImGui::Text("Camera position: %.2f %.2f %.2f", render_data.camera.position.x, render_data.camera.position.y, render_data.camera.position.z);

	// show camera facing
	ImGui::Text("Camera facing: %.2f %.2f %.2f", render_data.camera.front.x, render_data.camera.front.y, render_data.camera.front.z);

	ImGui::End();

	int res = draw_frame(init, render_data);
	if (res != 0) {
		std::cout << "failed to draw frame \n";
		return -1;
	}
}

void configure_mouse_input(const Init& init, RenderData& render_data) {
	glfwSetWindowUserPointer(init.window, &render_data);
	//glfwSetCursorPosCallback(init.window, mouse_callback);
	//glfwSetInputMode(init.window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}

int main() {
    Init init;
    RenderData render_data;

    if (0 != device_initialization(init)) return -1;
    if (0 != create_swapchain(init)) return -1;
    if (0 != create_render_pass(init, render_data)) return -1;
    if (0 != create_descriptor_set_layout(init, render_data)) return -1;
    if (0 != create_depth_resources(init, render_data)) return -1;
    if (0 != create_graphics_pipeline(init, render_data)) return -1;
    if (0 != create_framebuffers(init, render_data)) return -1;
    if (0 != create_command_pool(init, render_data)) return -1;
    if (0 != create_command_buffers(init, render_data)) return -1;
    if (0 != create_sync_objects(init, render_data)) return -1;
    if (0 != create_descriptor_pool(init, render_data)) return -1;
    if (0 != create_imgui(init, render_data)) return -1;
    if (0 != create_uniform_buffers(init, render_data)) return -1;

	init_shadow_pipeline(init, render_data);
	init_shadow_map(init, render_data);
    //render_data.texture = std::make_unique<Texture>( init, "../textures/wall.KTX2");

	render_data.staging_buffer = create_staging_buffer(init, 65000);

    ImageLoader* imageLoader = new ImageLoader(init);
    render_data.texture = imageLoader->load_texture("../textures/oldtruck_d.ktx2");
    render_data.cube_map_texture = imageLoader->load_cubemap("../textures/clouds.ktx2");
    render_data.cube_map = new CubeMap(init, render_data);

    if (0 != create_descriptor_sets(init, render_data)) return -1;
	render_data.mesh = Mesh::create_cube();
	render_data.mesh->transfer_mesh(init);
	render_data.plane_mesh = Mesh::create_plane(10, 10);
	render_data.plane_mesh->transfer_mesh(init);

    auto lastTime = std::chrono::high_resolution_clock::now();
    float deltaTime = 0.0f;

	configure_mouse_input(init, render_data);

	const auto cmdBuffer = begin_single_time_commands(init);
	transition_shadowmap_initial(init, cmdBuffer, render_data.shadow_map.image);
	end_single_time_commands(init, cmdBuffer);

	render_data.camera.position = glm::vec3(-2.2f, 1.66f, 1.7f);
	render_data.camera.look_at(glm::vec3(0.0f));

	// load the bunny model
	Mesh bunny_model = create_from_obj("../meshes/truck.obj");
	bunny_model.transfer_mesh(init);
	render_data.bunny_mesh = &bunny_model;

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

		create_new_imgui_frame(init, render_data);

    }
    init.disp.deviceWaitIdle();

    delete render_data.cube_map;
    delete imageLoader;

	cleanup_shadow_map(init, render_data);

	cleanup_buffer(init, render_data.staging_buffer);

    cleanup(init, render_data);
    return 0;
}