#pragma once

// Louron Core Headers
#include "Components/UUID.h"
#include "Resource Manager.h"
#include "../Renderer/RendererPipeline.h"

// C++ Standard Library Headers
#include <vector>
#include <memory>
#include <string>
#include <filesystem>

// External Vendor Library Headers
#include <entt/entt.hpp>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#define MAX_DIRECTIONAL_LIGHTS 10
#define MAX_POINT_LIGHTS 1024
#define MAX_SPOT_LIGHTS 1024

namespace Louron {

	class Entity;

	struct SceneConfig {

		std::string Name = "Untitled Scene";
		std::filesystem::path AssetDirectory;

		std::shared_ptr<RenderPipeline> ScenePipeline;
		std::shared_ptr<ResourceManager> SceneResourceManager;

		L_RENDER_PIPELINE ScenePipelineType;
	};

	class Scene : public std::enable_shared_from_this<Scene> {

	public:

		Scene();
		Scene(const std::filesystem::path& sceneFilePath, L_RENDER_PIPELINE pipelineType = L_RENDER_PIPELINE::FORWARD);
		~Scene() { }

	public:

		Entity CreateEntity(const std::string& name = std::string());
		Entity CreateEntity(UUID uuid, const std::string& name = std::string());

		Entity DuplicateEntity(Entity entity);
		void DestroyEntity(Entity entity);
		
		Entity FindEntityByName(std::string_view name);
		Entity FindEntityByUUID(UUID uuid);

		bool HasEntity(const std::string& name);

		bool IsRunning() const { return m_IsRunning; }
		bool IsPaused() const { return m_IsPaused; }

		void OnStart();
		void OnUpdate();
		void OnUpdateGUI();
		void OnStop();

		template<typename... Components>
		auto GetAllEntitiesWith() {	return m_Registry.view<Components...>(); }

		Entity GetPrimaryCameraEntity();

		void SetConfig(const SceneConfig& config) { m_SceneConfig = config; }
		const SceneConfig& GetConfig() const { return m_SceneConfig; }
		const std::shared_ptr<ResourceManager>& GetResources() const { return m_SceneConfig.SceneResourceManager; }

		entt::registry* GetRegistry() { return &m_Registry; }
		bool CopyRegistry(std::shared_ptr<Scene> otherScene);

	private:

		entt::registry m_Registry;
		std::unordered_map<UUID, entt::entity> m_EntityMap;

		bool m_IsRunning = false;
		bool m_IsPaused = false;

		std::filesystem::path m_SceneFilePath;
		SceneConfig m_SceneConfig;
		
		friend class Entity;
		friend class Project;
		friend class SceneSerializer;
	};
}