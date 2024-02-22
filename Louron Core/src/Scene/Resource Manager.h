#pragma once

// Louron Core Headers
#include "Components/Mesh.h"

#include "../OpenGL/Shader.h"
#include "../OpenGL/Texture.h"
#include "../OpenGL/Material.h"

#include "../Debug/Assert.h"

// C++ Standard Library Headers

// External Vendor Library Headers

namespace Louron {

	struct ResourceManager {

	public:

		std::unordered_map<std::string, std::shared_ptr<Shader>> Shaders;
		std::unordered_map<std::string, std::shared_ptr<Texture>> Textures;
		std::unordered_map<std::string, std::shared_ptr<Material>> Materials;

		std::unordered_map<std::string, std::pair<std::shared_ptr<MeshFilter>, std::shared_ptr<MeshRenderer>>> Meshes;

	public:

		int LoadMesh(const char* filePath, const std::shared_ptr<Shader>& shader);
		int LoadShader(const char* filePath);
		int LoadTexture(const char* filePath);

		void LinkShader(std::shared_ptr<Shader> shader);

		std::shared_ptr<MeshFilter> GetMeshFilter(const std::string& name);
		std::shared_ptr<MeshRenderer> GetMeshRenderer(const std::string& name);

		ResourceManager() = default;
		~ResourceManager() = default;

	private:

		void ProcessNode(aiNode* node, const aiScene* scene, std::pair<std::shared_ptr<MeshFilter>, std::shared_ptr<MeshRenderer>>& modelComponentGroup, const std::string& directory, const std::shared_ptr<Shader>& shader, const std::string& modelName);
		void ProcessMesh(aiMesh* mesh, const aiScene* scene, std::pair<std::shared_ptr<MeshFilter>, std::shared_ptr<MeshRenderer>>& modelComponentGroup, const std::string& directory, const std::shared_ptr<Shader>& shader, const std::string& modelName);

	};
}
