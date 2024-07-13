#pragma once

#include <ktxvulkan.h>

struct Init;

struct TextureImage {
    ktxVulkanTexture texture;
    VkSampler sampler;
    VkImageView view;
};

class ImageLoader {
public:
    explicit ImageLoader(Init& init);
    ~ImageLoader();

    TextureImage load_texture(const std::string ktxfile);
    TextureImage load_cubemap(const std::string ktxfile);

private:

    void cleanup_texture(TextureImage& texture);

    Init& init;
    std::vector<TextureImage> textures;

    ktxVulkanDeviceInfo kvdi;
};

