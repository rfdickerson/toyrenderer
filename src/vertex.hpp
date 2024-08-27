//
// Created by rfdic on 8/21/2024.
//

#ifndef VERTEX_HPP
#define VERTEX_HPP

#include <glm/glm.hpp>

struct Vertex
{
	glm::vec3 pos;
	glm::vec3 color;
	glm::vec2 tex_coord;
	glm::vec3 normal;
	glm::vec3 tangent;
	glm::vec3 bitangent;
};


#endif //VERTEX_HPP
