//
// Created by rfdic on 8/9/2024.
//

#include "obj_loader.hpp"

#include <mesh.hpp>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

namespace obsidian
{

Mesh create_from_obj(const std::string &file_path)
{
	Assimp::Importer importer;
	const aiScene   *scene = importer.ReadFile(file_path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);

	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	{
		std::cerr << "ERROR::ASSIMP::" << importer.GetErrorString() << std::endl;
	}

	Mesh mesh;

	// assiming the mesh contains only one mesh in the scene
	aiMesh *ai_mesh = scene->mMeshes[0];

	mesh.vertex_count = ai_mesh->mNumVertices;
	mesh.vertices.resize(mesh.vertex_count);

	for (unsigned int i = 0; i < mesh.vertex_count; ++i) {
		mesh.vertices[i].pos = {ai_mesh->mVertices[i].x, ai_mesh->mVertices[i].y, ai_mesh->mVertices[i].z};
		mesh.vertices[i].normal = {ai_mesh->mNormals[i].x, ai_mesh->mNormals[i].y, ai_mesh->mNormals[i].z};
		mesh.vertices[i].tex_coord = {ai_mesh->mTextureCoords[0][i].x, ai_mesh->mTextureCoords[0][i].y};
		mesh.vertices[i].normal = {ai_mesh->mNormals[i].x, ai_mesh->mNormals[i].y, ai_mesh->mNormals[i].z};
	}

	mesh.index_count = ai_mesh->mNumFaces * 3;
	mesh.indices.reserve(mesh.index_count);

	for (unsigned int i = 0; i < ai_mesh->mNumFaces; i++) {
		aiFace face = ai_mesh->mFaces[i];
		for (unsigned int j = 0; j < face.mNumIndices; j++) {
			mesh.indices.push_back(face.mIndices[j]);
		}
	}

	// show the mesh stats
	std::cout << "Vertices: " << mesh.vertex_count << std::endl;
    std::cout << "Indices: " << mesh.index_count << std::endl;

    // free the assimp resources
    importer.FreeScene();
    return mesh;

	return mesh;
}

}