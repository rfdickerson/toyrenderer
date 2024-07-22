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
	static Mesh* create_plane(uint32_t subdivisions, float size = 1.0f);

	VkResult draw(Init& init, VkCommandBuffer commandBuffer);
};

void create_cube_mesh(Init &init, Mesh& mesh);
void create_plane_mesh(Init &init, Mesh& mesh, uint32_t subdivisions, float size);
VkResult cleanup_mesh(Init &init, Mesh& mesh);

VkResult transfer_mesh(Init &init, Mesh &mesh);


}        // namespace obsidian

#endif        // TOYRENDERER_MESH_HPP
