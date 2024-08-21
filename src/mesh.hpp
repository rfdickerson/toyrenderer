#pragma once

#include "buffer.hpp"
#include "vertex.hpp"

namespace obsidian
{

struct Init;

using VertexBufferHandle = uint32_t;
using IndexBufferHandle = uint32_t;

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
	VkDescriptorSet descriptor_set;

	bool gpu_data_initialized = false;

};

struct UploadMeshDataStage
{
	BufferAllocation staging_buffer;
	BufferAllocation target_buffer;
	uint32_t src_offset;
	uint32_t dst_offset;
	uint32_t size;
};

struct UploadMeshData
{
	std::vector<UploadMeshDataStage> stages;
	uint32_t total_size;
};


BufferAllocation create_staging_buffer(Init &init, uint32_t size);

VkResult copy_buffer_data(Init &init, const VkCommandBuffer &command_buffer, const UploadMeshDataStage &stage);

void create_cube_mesh(Init &init, Mesh& mesh);

void create_plane_mesh(Init &init, Mesh& mesh, uint32_t subdivisions, float size);

VkResult cleanup_mesh(Init &init, Mesh& mesh);

VkResult transfer_mesh_to_gpu(Init &init, Mesh &mesh, BufferAllocation staging_buffer);



}        // namespace obsidian

