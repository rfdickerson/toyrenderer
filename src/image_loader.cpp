//
// Created by rfdic on 7/10/2024.
//

#include "image_loader.hpp"

Texture::Texture(Init &init, const std::string ktxfile): init(init) {
    ktxVulkanDeviceInfo kvdi;
    ktxTexture* kTexture;
    KTX_error_code ktxresult;

    ktxVulkanDeviceInfo_Construct(&kvdi,
                                  init.physical_device,
                                  init.device,
                                  init.graphics_queue,
                                  init.command_pool,
                                  nullptr);

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

    //ktxTexture_Destroy(kTexture);
    ktxVulkanDeviceInfo_Destruct(&kvdi);

}

Texture::~Texture() {
    vkDestroySampler(init.device, sampler, nullptr);
    vkDestroyImageView(init.device, view, nullptr);
    ktxVulkanTexture_Destruct(&texture, init.device, nullptr);
}