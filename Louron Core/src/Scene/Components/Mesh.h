#pragma once

// Louron Core Headers
#include "Camera.h"

#include "../../Core/Engine.h"

#include "../../OpenGL/Texture.h"
#include "../../OpenGL/Material.h"
#include "../../OpenGL/Vertex Array.h"

#include "Components.h"

// C++ Standard Library Headers
#include <string>
#include <vector>
#include <map>

// External Vendor Library Headers
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

	struct MeshFilter : public Component {

		std::shared_ptr<std::vector<std::shared_ptr<Mesh>>> Meshes = std::make_shared<std::vector<std::shared_ptr<Mesh>>>();

		void LinkMeshFilter(const std::shared_ptr<MeshFilter>& meshFilter);

		MeshFilter() = default;
		~MeshFilter() = default;

	};

	struct Transform;

	struct MeshRenderer : public Component {

	public:
		bool active = true;
		std::unordered_map<GLuint, std::shared_ptr<Material>> Materials{};

	public:

		int LoadModelFromFile(const char* filePath, MeshFilter& mesh);

		void LinkMeshRenderer(const std::shared_ptr<MeshRenderer>& meshRenderer);

		void ManualDraw(const MeshFilter& meshFilter, const Camera& camera, Transform& transform) const;

		void SetPath(const std::string& filePath);
		std::string GetPath() const;

		MeshRenderer() = default;
		MeshRenderer(const MeshRenderer&) = default;
		~MeshRenderer() = default;

	private:

		std::string m_Directory;
		std::string m_FilePath = "";

		void ProcessNode(aiNode* node, const aiScene* scene, MeshFilter& meshFilter, const std::string& directory, std::shared_ptr<Shader> shader, const std::string& modelName);
		void ProcessMesh(aiMesh* mesh, const aiScene* scene, MeshFilter& meshFilter, const std::string& directory, std::shared_ptr<Shader> shader, const std::string& modelName);
	};
}