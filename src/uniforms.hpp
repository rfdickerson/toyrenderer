//
// Created by rfdic on 8/21/2024.
//

#ifndef UNIFORMS_HPP
#define UNIFORMS_HPP

#include <glm/fwd.hpp>

namespace obsidian
{
struct UniformBufferObject
{
	glm::mat4 model;
	glm::mat4 view;
	glm::mat4 proj;
	glm::mat4 lightSpaceMatrix;
	glm::vec3 lightDirection;
	float nearPlane;
	float farPlane;
	float padding;
};
}

#endif //UNIFORMS_HPP
