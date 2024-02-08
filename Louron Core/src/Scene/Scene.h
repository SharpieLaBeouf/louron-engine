#pragma once

#include "Resource Manager.h"

#include <vector>
#include <memory>
#include <string>
#include <filesystem>

#include <entt/entt.hpp>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>


#define MAX_DIRECTIONAL_LIGHTS 10
#define MAX_POINT_LIGHTS 1024
#define MAX_SPOT_LIGHTS 1024

namespace Louron {

	class Entity;
	class RenderPipeline;

	struct SceneConfig {

		std::string Name = "Untitled Scene";
		std::filesystem::path AssetDirectory;
		std::shared_ptr<RenderPipeline> ScenePipeline;
		std::shared_ptr<ResourceManager> ResourceManager;
	};

	class Scene {

	public:

		Scene() = delete;
		Scene(const std::string& sceneName, std::shared_ptr<RenderPipeline> pipeline);
		~Scene() { }

		Entity CreateEntity(const std::string& name = std::string());
		Entity DuplicateEntity(Entity entity);
		void DestroyEntity(Entity entity);
		
		Entity FindEntityByName(std::string_view name);
		Entity GetPrimaryCameraEntity();

		bool HasEntity(const std::string& name);

		bool IsRunning() const { return m_IsRunning; }
		bool IsPaused() const { return m_IsPaused; }

		void OnStart();
		void OnUpdate();
		void OnUpdateGUI();
		void OnStop();

		template<typename... Components>
		auto GetAllEntitiesWith() {	return m_Registry.view<Components...>(); }

		const std::shared_ptr<ResourceManager>& GetResources() { return m_SceneConfig->ResourceManager; }

		entt::registry* GetRegistry() { return &m_Registry; }

	private:
		entt::registry m_Registry;
		bool m_IsRunning = false;
		bool m_IsPaused = false;

		std::unique_ptr<SceneConfig> m_SceneConfig = std::make_unique<SceneConfig>();
		
		friend class Entity;
		friend class SceneSerializer;
	};
}