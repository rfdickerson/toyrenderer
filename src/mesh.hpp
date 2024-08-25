#pragma once

#include "buffer.hpp"
#include "vertex.hpp"

namespace obsidian
{

struct Init;
struct RenderData;

using VertexBufferHandle = uint32_t;
using IndexBufferHandle = uint32_t;
using MaterialHandle = uint32_t;

// for creating the mesh data that will be loaded into the GPU
struct MeshData
{
	std::vector<Vertex> vertices;
	std::vector<uint16_t> indices;
};

struct TriangleSubmesh
{
	uint16_t index_offset;
	uint16_t index_count;
};

struct Mesh
{
  	VertexBufferHandle vertex_buffer_handle;
  	IndexBufferHandle index_buffer_handle;
	std::vector<TriangleSubmesh> submeshes;
	uint32_t material_index;

	bool gpu_data_initialized = false;

};

void draw_mesh(const Init &init, RenderData& render_data, VkCommandBuffer commandBuffer, const Mesh& mesh, uint32_t imageIndex);

MeshData create_plane(uint32_t subdivisions, float size); 

MeshData create_cube();

//void create_cube_mesh(Init &init, Mesh& mesh);
//
//void create_plane_mesh(Init &init, Mesh& mesh, uint32_t subdivisions, float size);

VkResult cleanup_mesh(Init &init, Mesh& mesh);

// VkResult transfer_mesh_to_gpu(Init &init, Mesh &mesh, BufferAllocation staging_buffer);



}        // namespace obsidian

