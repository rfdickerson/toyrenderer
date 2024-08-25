#pragma once

#include "image.hpp"
#include "image_loader.hpp"

namespace obsidian {

struct Material {
    TextureImage albedo_map;
    TextureImage normal_map;
    TextureImage metallic_roughness_map;
    TextureImage occlusion_map;

    float roughness;
    float metallic;
};

}