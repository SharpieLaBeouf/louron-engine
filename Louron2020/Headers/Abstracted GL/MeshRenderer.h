#pragma once
#include "../Entity.h"

#include <vector>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

struct Vertex {
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec3 texCoords;
};

class MeshFilter {

public:
	// mesh data
	std::vector<Vertex> m_Vertices;
	std::vector<GLuint> m_Indices;

	Shader* m_Shader = nullptr;
	std::vector<Material> m_Materials;

	MeshFilter(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Material> materials);
	
	void Draw();

private:

	void setupMesh();
};

class MeshRenderer : public Entity {

public:
	explicit MeshRenderer() = default;
	explicit MeshRenderer(const MeshRenderer&) = default;

	MeshRenderer(const char* filePath) {
		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile(filePath,
			aiProcess_CalcTangentSpace |
			aiProcess_Triangulate |
			aiProcess_JoinIdenticalVertices |
			aiProcess_SortByPType);
		if (!scene) {
			std::cout << "[L20] Error Reading File: " << importer.GetErrorString() << std::endl;
		}
	}

private:
	bool m_Active;
	std::vector<MeshFilter> m_Meshes;


};