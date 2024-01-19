#pragma once

#include <vector>
#include <memory>

#include "Mesh.h"
#include "../OpenGL/Shader.h"
#include "../OpenGL/Texture.h"
#include "../OpenGL/Material.h"

#include <entt/entt.hpp>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>


#define MAX_DIRECTIONAL_LIGHTS 10
#define MAX_POINT_LIGHTS 1024
#define MAX_SPOT_LIGHTS 1024

namespace Louron {

	class Entity;

	struct ResourceManager {

	public:

		std::unordered_map<std::string, std::shared_ptr<Shader>> Shaders;
		std::unordered_map<std::string, std::shared_ptr<Texture>> Textures;
		std::unordered_map<std::string, std::shared_ptr<Material>> Materials;
		
		std::unordered_map<std::string, std::vector<std::pair<std::shared_ptr<MeshFilter>, std::shared_ptr<Material>>>> Meshes;

	public:

		int LoadMesh(const char* filePath, std::shared_ptr<Shader> shader);
		int LoadShader(const char* filePath);
		int LoadTexture(const char* filePath);

		std::vector<std::pair<std::shared_ptr<MeshFilter>, std::shared_ptr<Material>>> GetMesh(const std::string& meshName);

		ResourceManager() = default;
		~ResourceManager() = default;
				
	private:

		std::vector<std::pair<std::shared_ptr<MeshFilter>, std::shared_ptr<Material>>> ProcessNode(aiNode* node, const aiScene* scene, std::string directory, std::shared_ptr<Shader> shader);
		std::pair<std::shared_ptr<MeshFilter>, std::shared_ptr<Material>> ProcessMesh(aiMesh* mesh, const aiScene* scene, std::string directory, std::shared_ptr<Shader> shader) const;

	};

	class Scene {

	public:

		Scene();
		~Scene() { }

		Entity CreateEntity(const std::string& name = std::string());
		Entity DuplicateEntity(Entity entity);
		void DestroyEntity(Entity entity);
		
		Entity FindEntityByName(std::string_view name);
		Entity GetPrimaryCameraEntity();

		bool IsRunning() const { return m_IsRunning; }
		bool IsPaused() const { return m_IsPaused; }

		void OnStart();
		void OnUpdate();
		void OnStop();

		template<typename... Components>
		auto GetAllEntitiesWith() {	return m_Registry.view<Components...>(); }

		const std::unique_ptr<ResourceManager>& GetResources() { return m_ResourceManager; }

	private:
		entt::registry m_Registry;
		bool m_IsRunning = false;
		bool m_IsPaused = false;

		std::unique_ptr<ResourceManager> m_ResourceManager = std::make_unique<ResourceManager>();

		struct ForwardPlusData {

			unsigned int PL_Buffer;
			unsigned int PL_Indices_Buffer;
			unsigned int SL_Buffer;
			unsigned int SL_Indices_Buffer;

			unsigned int DL_Buffer;

			unsigned int DepthMap_FBO;
			unsigned int DepthMap_Texture;

			unsigned int workGroupsX;
			unsigned int workGroupsY;

		} FP_Data;

		friend class Entity;
	};
}