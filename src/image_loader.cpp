//
// Created by rfdic on 7/10/2024.
//

#include "image_loader.hpp"
#include "common.hpp"

void ImageLoader::cleanup_texture(TextureImage &texture) {
    vkDestroySampler(init.device, texture.sampler, nullptr);
    vkDestroyImageView(init.device, texture.view, nullptr);
    ktxVulkanTexture_Destruct(&texture.texture, init.device, nullptr);
}

ImageLoader::ImageLoader(Init &init): init(init) {
    ktxVulkanDeviceInfo_Construct(&kvdi,
                                  init.physical_device,
                                  init.device,
                                  init.graphics_queue,
                                  init.command_pool,
                                  nullptr);
}

ImageLoader::~ImageLoader() {

    for (auto &texture : textures) {
        cleanup_texture(texture);
    }

    ktxVulkanDeviceInfo_Destruct(&kvdi);
}

TextureImage ImageLoader::load_texture(const std::string ktxfile) {
    ktxTexture* kTexture;
    KTX_error_code ktxresult;

    ktxVulkanTexture texture;
    VkSampler sampler;
    VkImageView view;

    ktxresult = ktxTexture_CreateFromNamedFile(ktxfile.c_str(),
                                               KTX_TEXTURE_CREATE_NO_FLAGS,
                                               &kTexture);

    if (KTX_SUCCESS != ktxresult) {
        std::stringstream message;
        message << "Creation of ktxTexture from file " << ktxfile << " failed: " << ktxErrorString(ktxresult);
        throw std::runtime_error(message.str());
    }

    ktxresult = ktxTexture_VkUploadEx(kTexture,
                                      &kvdi, &texture,
                                      VK_IMAGE_TILING_OPTIMAL,
                                      VK_IMAGE_USAGE_SAMPLED_BIT,
                                      VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    if (KTX_SUCCESS != ktxresult) {
        std::stringstream message;
        message << "Upload of ktxTexture to Vulkan device failed: " << ktxErrorString(ktxresult);
        throw std::runtime_error(message.str());
    }

    VkSamplerCreateInfo samplerInfo{
            .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
            .magFilter = VK_FILTER_LINEAR,
            .minFilter = VK_FILTER_LINEAR,
            .mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
            .addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT,
            .addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT,
            .addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT,
            .mipLodBias = 0.0f,
            .anisotropyEnable = VK_TRUE,
            .maxAnisotropy = 16,
            .compareEnable = VK_FALSE,
            .compareOp = VK_COMPARE_OP_ALWAYS,
            .minLod = 0.0f,
            .maxLod = static_cast<float>(kTexture->numLevels),
            .borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
            .unnormalizedCoordinates = VK_FALSE,
    };

    if (vkCreateSampler(init.device, &samplerInfo, nullptr, &sampler) != VK_SUCCESS) {
        throw std::runtime_error("failed to create texture sampler!");
    }

    VkImageViewCreateInfo viewInfo{

            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .image = texture.image,
            .viewType = texture.viewType,
            .format = texture.imageFormat,
            .subresourceRange = {
                    .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                    .baseMipLevel = 0,
                    .levelCount = texture.levelCount,
                    .baseArrayLayer = 0,
                    .layerCount = texture.layerCount,
            },
    };

    if (vkCreateImageView(init.device, &viewInfo, nullptr, &view) != VK_SUCCESS) {
        throw std::runtime_error("failed to create texture image view!");
    }

    // print a summary of the texture
    std::cout << "Texture Summary:" << std::endl;
    std::cout << "  " << ktxfile << std::endl;
    std::cout << "  " << "Dimensions: " << texture.width << "x" << texture.height << "x" << texture.depth << std::endl;
    std::cout << "  " << "Format: " << texture.imageFormat << std::endl;
    std::cout << "  " << "Mip Levels: " << texture.levelCount << std::endl;
    std::cout << "  " << "Array Layers: " << texture.layerCount << std::endl;

    ktxTexture_Destroy(kTexture);

    TextureImage newTexture;
    newTexture.texture = texture;
    newTexture.sampler = sampler;
    newTexture.view = view;

    textures.push_back(newTexture);

    return newTexture;
}

TextureImage ImageLoader::load_cubemap(const std::string ktxfile) {
    ktxTexture2* kTexture;
    KTX_error_code ktxresult;

    ktxVulkanTexture texture;
    VkSampler sampler;
    VkImageView view;

    ktxresult = ktxTexture2_CreateFromNamedFile(ktxfile.c_str(),
                                               KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT,
                                               &kTexture);

    if (KTX_SUCCESS != ktxresult) {
        std::stringstream message;
        message << "Creation of ktxTexture from file " << ktxfile << " failed: " << ktxErrorString(ktxresult);
        throw std::runtime_error(message.str());
    }

    if (ktxTexture2_NeedsTranscoding(kTexture)) {
        ktxresult = ktxTexture2_TranscodeBasis(kTexture, KTX_TTF_BC1_RGB, 0);
        if (KTX_SUCCESS != ktxresult) {
            std::stringstream message;
            message << "Transcoding of ktxTexture from file " << ktxfile << " failed: " << ktxErrorString(ktxresult);
            throw std::runtime_error(message.str());
        }
    }

    ktxresult = ktxTexture2_VkUploadEx(kTexture,
                                      &kvdi, &texture,
                                      VK_IMAGE_TILING_OPTIMAL,
                                      VK_IMAGE_USAGE_SAMPLED_BIT,
                                      VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    if (KTX_SUCCESS != ktxresult) {
        std::stringstream message;
        message << "Upload of ktxTexture to Vulkan device failed: " << ktxErrorString(ktxresult);
        throw std::runtime_error(message.str());
    }

    VkSamplerCreateInfo samplerInfo{
            .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
            .magFilter = VK_FILTER_LINEAR,
            .minFilter = VK_FILTER_LINEAR,
            .mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
            .addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
            .addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
            .addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
            .mipLodBias = 0.0f,
            .anisotropyEnable = VK_TRUE,
            .maxAnisotropy = 16,
            .compareEnable = VK_FALSE,
            .compareOp = VK_COMPARE_OP_ALWAYS,
            .minLod = 0.0f,
            .maxLod = static_cast<float>(kTexture->numLevels),
            .borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
            .unnormalizedCoordinates = VK_FALSE,
    };

    if (vkCreateSampler(init.device, &samplerInfo, nullptr, &sampler) != VK_SUCCESS) {
        throw std::runtime_error("failed to create texture sampler!");
    }

    VkImageViewCreateInfo viewInfo{
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .image = texture.image,
            .viewType = texture.viewType,
            .format = texture.imageFormat,
            .subresourceRange = {
                    .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                    .baseMipLevel = 0,
                    .levelCount = texture.levelCount,
                    .baseArrayLayer = 0,
                    .layerCount = texture.layerCount,
            },
    };

    if (vkCreateImageView(init.device, &viewInfo, nullptr, &view) != VK_SUCCESS) {
        throw std::runtime_error("failed to create texture image view!");
    }

}