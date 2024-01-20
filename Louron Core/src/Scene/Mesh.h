#pragma once

#include "../OpenGL/Texture.h"
#include "../OpenGL/Material.h"
#include "../OpenGL/Vertex Array.h"

#include "../Core/Engine.h"

#include <string>
#include <vector>
#include <map>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

namespace Louron {

	struct Mesh {

		std::unique_ptr<VertexArray> VAO;
		GLuint MaterialIndex = -1;

		Mesh() = default;
		Mesh(const std::vector<Vertex>& vertices, const std::vector<GLuint>& indices);
		~Mesh() = default;

	};

	struct MeshFilter {

		std::shared_ptr<std::vector<std::shared_ptr<Mesh>>> Meshes = std::make_shared<std::vector<std::shared_ptr<Mesh>>>();

		void LinkMeshFilterFromScene(const std::shared_ptr<MeshFilter>& meshFilter);

		MeshFilter() = default;
		~MeshFilter() = default;

	};

	struct MeshRenderer {

	public:
		bool active = true;
		std::shared_ptr<std::vector<std::shared_ptr<Material>>> Materials = std::make_shared<std::vector<std::shared_ptr<Material>>>();

	public:

		int LoadModelFromFile(const char* filePath, MeshFilter& mesh);

		void LinkMeshRendererFromScene(const std::shared_ptr<MeshRenderer>& meshRenderer);

		MeshRenderer() = default;
		MeshRenderer(const MeshRenderer&) = default;
		~MeshRenderer() = default;

	private:

		std::string m_Directory;

		void ProcessNode(aiNode* node, const aiScene* scene, MeshFilter& meshFilter);
		std::shared_ptr<Mesh> ProcessMesh(aiMesh* mesh, const aiScene* scene) const;
	};
}