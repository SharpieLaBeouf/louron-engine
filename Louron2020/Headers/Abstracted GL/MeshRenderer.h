#pragma once
#include "../Entity.h"
#include "../InstanceManager.h"

#include "Material.h"

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

	Material* m_Material;

	MeshFilter(std::vector<Vertex> vertices, std::vector<unsigned int> indices, Material* material);
	
	/// <summary>
	/// 1. DrawElements for the entire mesh using the singular material
	/// </summary>
	void Draw() { 
		
	}

private:

	void setupMesh() { }
};

class MeshRenderer : public Entity {

public:
	explicit MeshRenderer() = default;
	explicit MeshRenderer(const MeshRenderer&) = default;

	MeshRenderer(State::InstanceManager* instanceManager, bool active = true) : m_Active(active) {

	}

	int loadModel(const char* filePath) {

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

	/// <summary>
	/// 1. Sorts all meshfilters based on material
	/// 2. Loops all different types of materials
	///		-> Bind Material00 (set uniforms, etc.)
	///			-> Loop render all meshes with bound material00
	///		-> Bind Material01 (set uniforms, etc.)
	///			-> Loop render all meshes with bound material01
	///		-> etc ... foreach groupings of material
	/// </summary>
	void Draw() {

	}

private:
	bool m_Active;
	std::vector<MeshFilter> m_Meshes;


};