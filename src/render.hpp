//
// Created by rfdic on 8/20/2024.
//

#ifndef RENDER_HPP
#define RENDER_HPP

#include "mesh.hpp"

namespace obsidian
{

struct Light {
    glm::vec3 position;
    glm::vec3 color;
    float intensity;
};

struct Scene {
    std::vector<Mesh> meshes;
    std::vector<Light> lights;
};


}

#endif //RENDER_HPP
