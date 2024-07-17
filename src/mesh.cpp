//
// Created by rfdic on 7/15/2024.
//

#include "mesh.hpp"

#include "common.hpp"
#include "utils.hpp"

namespace obsidian
{


Mesh* Mesh::create_cube()
{
	const std::vector<Vertex> vertices = {
	    // Front face
	    {{-0.5f, -0.5f,  0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}, {0.0f, 0.0f,  1.0f}}, // Bottom-left
	    {{ 0.5f, -0.5f,  0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}, {0.0f, 0.0f,  1.0f}}, // Bottom-right
	    {{ 0.5f,  0.5f,  0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}, {0.0f, 0.0f,  1.0f}}, // Top-right
	    {{-0.5f,  0.5f,  0.5f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}, {0.0f, 0.0f,  1.0f}}, // Top-left

	    // Back face
	    {{-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}, {0.0f, 0.0f, -1.0f}}, // Bottom-left
	    {{ 0.5f, -0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}, {0.0f, 0.0f, -1.0f}}, // Bottom-right
	    {{ 0.5f,  0.5f, -0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}, {0.0f, 0.0f, -1.0f}}, // Top-right
	    {{-0.5f,  0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}, {0.0f, 0.0f, -1.0f}}, // Top-left

	    // Top face
	    {{-0.5f,  0.5f,  0.5f}, {1.0f, 0.0f, 1.0f}, {0.0f, 0.0f}, {0.0f, 1.0f,  0.0f}}, // Front-left
	    {{ 0.5f,  0.5f,  0.5f}, {0.0f, 1.0f, 1.0f}, {1.0f, 0.0f}, {0.0f, 1.0f,  0.0f}}, // Front-right
	    {{ 0.5f,  0.5f, -0.5f}, {1.0f, 1.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f,  0.0f}}, // Back-right
	    {{-0.5f,  0.5f, -0.5f}, {0.0f, 0.0f, 0.0f}, {0.0f, 1.0f}, {0.0f, 1.0f,  0.0f}}, // Back-left

	    // Bottom face
	    {{-0.5f, -0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f}, {0.0f, -1.0f,  0.0f}}, // Back-left
	    {{ 0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 1.0f}, {1.0f, 1.0f}, {0.0f, -1.0f,  0.0f}}, // Back-right
	    {{ 0.5f, -0.5f,  0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}, {0.0f, -1.0f,  0.0f}}, // Front-right
	    {{-0.5f, -0.5f,  0.5f}, {0.0f, 1.0f, 1.0f}, {0.0f, 0.0f}, {0.0f, -1.0f,  0.0f}}, // Front-left

	    // Right face
	    {{ 0.5f, -0.5f,  0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}, {1.0f,  0.0f,  0.0f}}, // Bottom-front
	    {{ 0.5f,  0.5f,  0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}, {1.0f,  0.0f,  0.0f}}, // Top-front
	    {{ 0.5f,  0.5f, -0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}, {1.0f,  0.0f,  0.0f}}, // Top-back
	    {{ 0.5f, -0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}, {1.0f,  0.0f,  0.0f}}, // Bottom-back

	    // Left face
	    {{-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {1.0f, 1.0f}, {-1.0f,  0.0f,  0.0f}}, // Bottom-back
	    {{-0.5f,  0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}, {-1.0f,  0.0f,  0.0f}}, // Top-back
	    {{-0.5f,  0.5f,  0.5f}, {1.0f, 1.0f, 0.0f}, {0.0f, 0.0f}, {-1.0f,  0.0f,  0.0f}}, // Top-front
	    {{-0.5f, -0.5f,  0.5f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}, {-1.0f,  0.0f,  0.0f}}  // Bottom-front
	};

	const std::vector<uint16_t> indices = {
	    // Front face
	    0, 1, 2,    // First triangle (bottom-left to top-right)
	    2, 3, 0,    // Second triangle (top-right to top-left)

	    // Back face
	    5, 4, 7,
	    7, 6, 5,

	    // Top face
	    8, 9, 10,
	    10, 11, 8,

	    // Bottom face
	    12, 13, 14,
	    14, 15, 12,

	    // Right face
	    17, 16, 19,
	    19, 18, 17,

	    // Left face
	    20, 21, 22,
	    22, 23, 20
	};

	Mesh* mesh = new Mesh();
	mesh->mesh_type = MeshType::CUBE;
	mesh->vertex_count = static_cast<uint32_t>(vertices.size());
	mesh->index_count = static_cast<uint32_t>(indices.size());

	mesh->vertices = vertices;
	mesh->indices = indices;

	return mesh;
}

VkResult Mesh::transfer_mesh(Init &init)
{
	VkDeviceSize vertex_buffer_size = sizeof(vertices[0]) * vertices.size();
	VkDeviceSize index_buffer_size = sizeof(indices[0]) * indices.size();

	BufferAllocation staging_vertex_buffer;
	BufferAllocation staging_index_buffer;

	create_buffer(init, vertex_buffer_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY, staging_vertex_buffer);
	create_buffer(init, index_buffer_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY, staging_index_buffer);

	void *data;
	vmaMapMemory(init.allocator, staging_vertex_buffer.allocation, &data);
	memcpy(data, vertices.data(), static_cast<size_t>(vertex_buffer_size));
	vmaUnmapMemory(init.allocator, staging_vertex_buffer.allocation);

	vmaMapMemory(init.allocator, staging_index_buffer.allocation, &data);
	memcpy(data, indices.data(), static_cast<size_t>(index_buffer_size));
	vmaUnmapMemory(init.allocator, staging_index_buffer.allocation);

	create_buffer(init, vertex_buffer_size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VMA_MEMORY_USAGE_GPU_ONLY, vertex_buffer);
	create_buffer(init, index_buffer_size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VMA_MEMORY_USAGE_GPU_ONLY, index_buffer);

	copy_buffer(init, staging_vertex_buffer.buffer, vertex_buffer.buffer, vertex_buffer_size);
	copy_buffer(init, staging_index_buffer.buffer, index_buffer.buffer, index_buffer_size);

	cleanup_buffer(init, staging_vertex_buffer);
	cleanup_buffer(init, staging_index_buffer);

	gpu_data_initialized = true;

	return VK_SUCCESS;
}

VkResult Mesh::draw(obsidian::Init &init, VkCommandBuffer commandBuffer)
{
	if (!gpu_data_initialized)
	{
		return VK_NOT_READY;
	}

	VkBuffer vertex_buffers[] = {vertex_buffer.buffer};
	VkDeviceSize offsets[] = {0};

	// bind vertex buffer
	init.disp.cmdBindVertexBuffers(commandBuffer, 0, 1, vertex_buffers, offsets);
	init.disp.cmdBindIndexBuffer(commandBuffer, index_buffer.buffer, 0, VK_INDEX_TYPE_UINT16);

	init.disp.cmdDrawIndexed(commandBuffer, static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);

	return VK_SUCCESS;
}



}        // namespace obsidian