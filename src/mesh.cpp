//
// Created by rfdic on 7/15/2024.
//

#include "mesh.hpp"

#include "common.hpp"
#include "uniforms.hpp"
#include "utils.hpp"

namespace obsidian
{


MeshData create_cube()
{
	const std::vector<Vertex> vertices {
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

	const std::vector<uint16_t> cube_indices {
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

	return MeshData {
		.vertices = vertices,
		.indices = cube_indices,
	};
}

MeshData create_plane(uint32_t subdivisions, float size)
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

	return MeshData {
		.vertices = vertices,
		.indices = indices,
	};
}

void draw_mesh(const Init &init, RenderData& render_data, VkCommandBuffer commandBuffer, const Mesh& mesh, uint32_t imageIndex)
{

	const BufferAllocation vertex_buffer = render_data.vertex_buffers[mesh.vertex_buffer_handle];
	const BufferAllocation index_buffer = render_data.index_buffers[mesh.index_buffer_handle];
	VkDescriptorSet descriptor_set = render_data.descriptor_sets[imageIndex];

	const VkBuffer         vertex_buffers[] = {vertex_buffer.buffer};
	constexpr VkDeviceSize offsets[] = {0};

	// bind vertex buffer
	init.disp.cmdBindVertexBuffers(commandBuffer, 0, 1, vertex_buffers, offsets);
	init.disp.cmdBindIndexBuffer(commandBuffer, index_buffer.buffer, 0, VK_INDEX_TYPE_UINT16);

	init.disp.cmdBindDescriptorSets(commandBuffer,
		VK_PIPELINE_BIND_POINT_GRAPHICS,
		render_data.pipeline_layout,
		0, 1,
		&descriptor_set,
		0, nullptr);

	// set the push constants
	const PushConstantBuffer push_constants = {1.0f, true};
	init.disp.cmdPushConstants(commandBuffer, render_data.pipeline_layout, VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PushConstantBuffer), &push_constants);


	// loop over the submeshes
	for (auto submesh : mesh.submeshes)
	{
		init.disp.cmdDrawIndexed(commandBuffer, submesh.index_count, 1, 0, 0, 0);
	}

}

VkResult cleanup_mesh(Init &init, Mesh& mesh)
{

	return VK_SUCCESS;
}



// VkResult copy_buffer_data(Init &init,
//                           const VkCommandBuffer &commandBuffer,
//                           BufferAllocation &src_buffer,
//                           BufferAllocation &dst_buffer,
//                           uint32_t size, uint32_t src_offset, uint32_t dst_offset)
// {
// 	VkBufferCopy copyRegion = {};
// 	copyRegion.size = size;
// 	copyRegion.srcOffset = src_offset;
// 	copyRegion.dstOffset = dst_offset;
// 	init.disp.cmdCopyBuffer(commandBuffer, src_buffer.buffer, dst_buffer.buffer, 1, &copyRegion);

// 	return VK_SUCCESS;
// }


}        // namespace obsidian