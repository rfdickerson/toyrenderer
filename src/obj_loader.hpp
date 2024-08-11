//
// Created by rfdic on 8/9/2024.
//

#ifndef TOYRENDERER_OBJ_LOADER_HPP
#define TOYRENDERER_OBJ_LOADER_HPP

#include "mesh.hpp"

namespace obsidian {

Mesh create_from_obj(const std::string& filename);

}

#endif        // TOYRENDERER_OBJ_LOADER_HPP
