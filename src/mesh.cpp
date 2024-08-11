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
	const std::vector<Vertex> vertices ={
	    // Front face (Z+)
	    {{-0.5f, -0.5f,  0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}, {0.0f, 0.0f,  1.0f}}, // 0. Bottom-left
	    {{ 0.5f, -0.5f,  0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}, {0.0f, 0.0f,  1.0f}}, // 1. Bottom-right
	    {{ 0.5f,  0.5f,  0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}, {0.0f, 0.0f,  1.0f}}, // 2. Top-right
	    {{-0.5f,  0.5f,  0.5f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}, {0.0f, 0.0f,  1.0f}}, // 3. Top-left

	    // Back face (Z-)
	    {{-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}, {0.0f, 0.0f, -1.0f}}, // 4. Bottom-left
	    {{ 0.5f, -0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}, {0.0f, 0.0f, -1.0f}}, // 5. Bottom-right
	    {{ 0.5f,  0.5f, -0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}, {0.0f, 0.0f, -1.0f}}, // 6. Top-right
	    {{-0.5f,  0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}, {0.0f, 0.0f, -1.0f}}, // 7. Top-left

	    // Top face
	    {{-0.5f,  0.5f,  0.5f}, {1.0f, 0.0f, 1.0f}, {0.0f, 1.0f}, {0.0f, 1.0f,  0.0f}}, // 8. Front-left
	    {{ 0.5f,  0.5f,  0.5f}, {0.0f, 1.0f, 1.0f}, {1.0f, 1.0f}, {0.0f, 1.0f,  0.0f}}, // 9. Front-right
	    {{ 0.5f,  0.5f, -0.5f}, {1.0f, 1.0f, 0.0f}, {1.0f, 0.0f}, {0.0f, 1.0f,  0.0f}}, // 10. Back-right
	    {{-0.5f,  0.5f, -0.5f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f}, {0.0f, 1.0f,  0.0f}}, // 11. Back-left

	    // Bottom face
	    {{-0.5f, -0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f}, {0.0f, -1.0f,  0.0f}}, // 12. Back-left
	    {{ 0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 1.0f}, {1.0f, 1.0f}, {0.0f, -1.0f,  0.0f}}, // 13. Back-right
	    {{ 0.5f, -0.5f,  0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}, {0.0f, -1.0f,  0.0f}}, // 14. Front-right
	    {{-0.5f, -0.5f,  0.5f}, {0.0f, 1.0f, 1.0f}, {0.0f, 0.0f}, {0.0f, -1.0f,  0.0f}}, // 15. Front-left

	    // Right face
	    {{ 0.5f, -0.5f,  0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}, {1.0f,  0.0f,  0.0f}}, // 16. Bottom-front
	    {{ 0.5f,  0.5f,  0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}, {1.0f,  0.0f,  0.0f}}, // 17. Top-front
	    {{ 0.5f,  0.5f, -0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}, {1.0f,  0.0f,  0.0f}}, // 18. Top-back
	    {{ 0.5f, -0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}, {1.0f,  0.0f,  0.0f}}, // 19. Bottom-back

	    // Left face (X-)
	    {{-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}, {-1.0f,  0.0f,  0.0f}}, // 20. Bottom-back
	    {{-0.5f,  0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f}, {-1.0f,  0.0f,  0.0f}}, // 21. Top-back
	    {{-0.5f,  0.5f,  0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}, {-1.0f,  0.0f,  0.0f}}, // 22. Top-front
	    {{-0.5f, -0.5f,  0.5f}, {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f}, {-1.0f,  0.0f,  0.0f}}  // 23. Bottom-front
	};

	const std::vector<uint16_t> cube_indices = {
	    // Front face
	    1, 2, 0,    2, 3, 0,
	    // Back face
	    4, 6, 5,    6, 4, 7,
	    // Top face (good)
	    8, 9, 10,   10, 11, 8,
	    // Bottom face (good)
	    12, 13, 14, 14, 15, 12,
	    // Right face
	    16, 18, 17, 18, 16, 19,
	    // Left face
	    20, 22, 21, 22, 20, 23
	};

	Mesh* mesh = new Mesh();
	mesh->mesh_type = MeshType::CUBE;
	mesh->vertex_count = static_cast<uint32_t>(vertices.size());
	mesh->index_count = static_cast<uint32_t>(cube_indices.size());

	mesh->vertices = vertices;
	mesh->indices = cube_indices;

	return mesh;
}

Mesh* Mesh::create_plane(uint32_t subdivisions, float size)
{
	// Calculate the number of vertices and indices
	uint32_t numVertices = (subdivisions + 1) * (subdivisions + 1);
	uint32_t numIndices = subdivisions * subdivisions * 6;

	// Generate the vertices
	std::vector<Vertex> vertices(numVertices);
	for (uint32_t y = 0; y <= subdivisions; y++) {
		for (uint32_t x = 0; x <= subdivisions; x++) {
			float u = static_cast<float>(x) / subdivisions;
			float v = static_cast<float>(y) / subdivisions;
			vertices[y * (subdivisions + 1) + x] = {
			    {-size / 2.0f + u * size, 0.0f, -size / 2.0f + v * size},
			    {1.0f, 1.0f, 1.0f},
			    {u, v},
			    {0.0f, 1.0f, 0.0f}
			};
		}
	}

	// Generate the indices
	std::vector<uint16_t> indices(numIndices);
	for (uint32_t y = 0; y < subdivisions; y++) {
		for (uint32_t x = 0; x < subdivisions; x++) {
			uint16_t topLeft = y * (subdivisions + 1) + x;
			uint16_t topRight = topLeft + 1;
			uint16_t bottomLeft = (y + 1) * (subdivisions + 1) + x;
			uint16_t bottomRight = bottomLeft + 1;

			uint32_t indexOffset = (y * subdivisions + x) * 6;
			indices[indexOffset + 0] = topLeft;
			indices[indexOffset + 1] = bottomLeft;
			indices[indexOffset + 2] = topRight;
			indices[indexOffset + 3] = topRight;
			indices[indexOffset + 4] = bottomLeft;
			indices[indexOffset + 5] = bottomRight;
		}
	}

	// Create the mesh
	Mesh* mesh = new Mesh();
	mesh->mesh_type = MeshType::PLANE;
	mesh->vertex_count = numVertices;
	mesh->index_count = numIndices;
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

//const std::vector<Vertex> cube_vertices = {
//    // Front face
//    {{-0.5f, -0.5f,  0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}, {0.0f, 0.0f,  1.0f}}, // Bottom-left
//    {{ 0.5f, -0.5f,  0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}, {0.0f, 0.0f,  1.0f}}, // Bottom-right
//    {{ 0.5f,  0.5f,  0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}, {0.0f, 0.0f,  1.0f}}, // Top-right
//    {{-0.5f,  0.5f,  0.5f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}, {0.0f, 0.0f,  1.0f}}, // Top-left
//
//    // Back face
//    {{-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}, {0.0f, 0.0f, -1.0f}}, // Bottom-left
//    {{ 0.5f, -0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}, {0.0f, 0.0f, -1.0f}}, // Bottom-right
//    {{ 0.5f,  0.5f, -0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}, {0.0f, 0.0f, -1.0f}}, // Top-right
//    {{-0.5f,  0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}, {0.0f, 0.0f, -1.0f}}, // Top-left
//
//    // Top face
//    {{-0.5f,  0.5f,  0.5f}, {1.0f, 0.0f, 1.0f}, {0.0f, 0.0f}, {0.0f, 1.0f,  0.0f}}, // Front-left
//    {{ 0.5f,  0.5f,  0.5f}, {0.0f, 1.0f, 1.0f}, {1.0f, 0.0f}, {0.0f, 1.0f,  0.0f}}, // Front-right
//    {{ 0.5f,  0.5f, -0.5f}, {1.0f, 1.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f,  0.0f}}, // Back-right
//    {{-0.5f,  0.5f, -0.5f}, {0.0f, 0.0f, 0.0f}, {0.0f, 1.0f}, {0.0f, 1.0f,  0.0f}}, // Back-left
//
//    // Bottom face
//    {{-0.5f, -0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f}, {0.0f, -1.0f,  0.0f}}, // Back-left
//    {{ 0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 1.0f}, {1.0f, 1.0f}, {0.0f, -1.0f,  0.0f}}, // Back-right
//    {{ 0.5f, -0.5f,  0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}, {0.0f, -1.0f,  0.0f}}, // Front-right
//    {{-0.5f, -0.5f,  0.5f}, {0.0f, 1.0f, 1.0f}, {0.0f, 0.0f}, {0.0f, -1.0f,  0.0f}}, // Front-left
//
//    // Right face
//    {{ 0.5f, -0.5f,  0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}, {1.0f,  0.0f,  0.0f}}, // Bottom-front
//    {{ 0.5f,  0.5f,  0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}, {1.0f,  0.0f,  0.0f}}, // Top-front
//    {{ 0.5f,  0.5f, -0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}, {1.0f,  0.0f,  0.0f}}, // Top-back
//    {{ 0.5f, -0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}, {1.0f,  0.0f,  0.0f}}, // Bottom-back
//
//    // Left face
//    {{-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {1.0f, 1.0f}, {-1.0f,  0.0f,  0.0f}}, // Bottom-back
//    {{-0.5f,  0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}, {-1.0f,  0.0f,  0.0f}}, // Top-back
//    {{-0.5f,  0.5f,  0.5f}, {1.0f, 1.0f, 0.0f}, {0.0f, 0.0f}, {-1.0f,  0.0f,  0.0f}}, // Top-front
//    {{-0.5f, -0.5f,  0.5f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}, {-1.0f,  0.0f,  0.0f}}  // Bottom-front
//};
//
//const std::vector<uint16_t> cube_indices = {
//    // Front face
//    0, 1, 2,    // First triangle (bottom-left, bottom-right, top-right)
//    2, 3, 0,    // Second triangle (top-right, top-left, bottom-left)
//
//    // Back face
//    4, 6, 5,    // First triangle (bottom-left, top-right, bottom-right)
//    6, 4, 7,    // Second triangle (top-right, bottom-left, top-left)
//
//    // Top face
//    9, 9, 10,   // First triangle (front-left, front-right, back-right)
//    10, 11, 8,  // Second triangle (back-right, back-left, front-left)
//
//    // Bottom face
//    12, 15, 13, // First triangle (back-left, front-left, back-right)
//    15, 14, 13, // Second triangle (front-left, front-right, back-right)
//
//    // Right face
//    16, 17, 18, // First triangle (bottom-front, top-front, top-back)
//    18, 19, 16, // Second triangle (top-back, bottom-back, bottom-front)
//
//    // Left face
//    20, 23, 21, // First triangle (bottom-back, bottom-front, top-back)
//    23, 22, 21  // Second triangle (bottom-front, top-front, top-back)
//};

//void create_cube_mesh(Mesh& mesh) {
//	mesh.mesh_type = MeshType::CUBE;
//	mesh.vertices = cube_vertices;
//	mesh.indices = cube_indices;
//}

VkResult cleanup_mesh(Init &init, Mesh& mesh)
{
	cleanup_buffer(init, mesh.vertex_buffer);
	cleanup_buffer(init, mesh.index_buffer);
	return VK_SUCCESS;
}

VkResult transfer_mesh_to_gpu(Init& init, Mesh& mesh, BufferAllocation staging_buffer)
{

	// copy vertex data to staging buffer
	VkDeviceSize vertex_buffer_size = sizeof(mesh.vertices[0]) * mesh.vertices.size();
	VkDeviceSize index_buffer_size = sizeof(mesh.indices[0]) * mesh.indices.size();

	return VK_SUCCESS;

}
BufferAllocation create_staging_buffer(Init &init, uint32_t size)
{
	BufferAllocation staging_buffer;

	VkBufferCreateInfo buffer_info = {
	    .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
	    .size = size,
	    .usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
	    .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
	};

	VmaAllocationCreateInfo alloc_info = {
	    .flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT |
	             VMA_ALLOCATION_CREATE_MAPPED_BIT,
	    .usage = VMA_MEMORY_USAGE_AUTO,
	};

	if (vmaCreateBuffer(init.allocator, &buffer_info, &alloc_info, &staging_buffer.buffer, &staging_buffer.allocation, nullptr) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create staging buffer!");
	}

	return staging_buffer;
}

VkResult copy_buffer_data(Init &init,
                          const VkCommandBuffer &commandBuffer,
                          BufferAllocation &src_buffer,
                          BufferAllocation &dst_buffer,
                          uint32_t size, uint32_t src_offset, uint32_t dst_offset)
{
	VkBufferCopy copyRegion = {};
	copyRegion.size = size;
	copyRegion.srcOffset = src_offset;
	copyRegion.dstOffset = dst_offset;
	init.disp.cmdCopyBuffer(commandBuffer, src_buffer.buffer, dst_buffer.buffer, 1, &copyRegion);

	return VK_SUCCESS;
}


}        // namespace obsidian