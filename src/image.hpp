//
// Created by rfdic on 8/20/2024.
//

#ifndef IMAGE_HPP
#define IMAGE_HPP

namespace obsidian {

struct Init;

struct Image
{
	VkImage image;
	VkImageView imageView;
	VmaAllocation allocation;
	VkFormat format;
};

Image create_depth_image(const Init& init, VkFormat format);

void cleanup_image(const Init& init, Image& image);

void transition_image_to_color_attachment(const Init& init, VkCommandBuffer command_buffer, VkImage image);

void transition_image_to_depth_attachment(Init &init, const VkCommandBuffer& command_buffer, const VkImage& image);

void transition_image_to_present(Init &init, const VkCommandBuffer& command_buffer, const VkImage& image);

void transition_shadowmap_initial(Init &init, const VkCommandBuffer& command_buffer, const VkImage& image);

void transition_shadowmap_to_shader_read(Init &init, const VkCommandBuffer& command_buffer, const VkImage& image);

void transition_shadowmap_to_depth_attachment(Init &init, const VkCommandBuffer& command_buffer, const VkImage& image);

}

#endif //IMAGE_HPP
