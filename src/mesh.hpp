//
// Created by rfdic on 7/15/2024.
//

#ifndef TOYRENDERER_MESH_HPP
#define TOYRENDERER_MESH_HPP

#include "common.hpp"

namespace obsidian
{

enum class MeshType {
  CUBE,
  SPHERE,
  PLANE,
  CUSTOM
};

struct Mesh
{
	MeshType mesh_type;
	uint32_t vertex_count;
	uint32_t index_count;

	std::vector<Vertex> vertices;
	std::vector<uint16_t> indices;

  	BufferAllocation vertex_buffer;
  	BufferAllocation index_buffer;

	bool gpu_data_initialized = false;

	VkResult transfer_mesh(Init &init);

	static Mesh* create_cube();
	VkResult draw(Init& init, VkCommandBuffer commandBuffer);
};

}        // namespace obsidian

#endif        // TOYRENDERER_MESH_HPP
