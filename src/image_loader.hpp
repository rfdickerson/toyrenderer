//
// Created by rfdic on 7/10/2024.
//

#ifndef TOYRENDERER_IMAGE_LOADER_HPP
#define TOYRENDERER_IMAGE_LOADER_HPP

#include "common.hpp"
#include <ktxvulkan.h>

class Texture {
public:
    Texture(Init &init, const std::string ktxfile);
    ~Texture();

    Init& init;
    ktxVulkanTexture texture;

    VkSampler sampler;
    VkImageView view;

};


#endif //TOYRENDERER_IMAGE_LOADER_HPP
