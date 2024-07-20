#pragma once

// Louron Core Headers
#include "../Core/Logging.h"
#include "../Asset/Asset.h"
#include "Components/Physics/CollisionCallback.h"
#include "Components/Components.h"

// C++ Standard Library Headers
#include <vector>
#include <memory>
#include <optional>
#include <string>
#include <filesystem>

// External Vendor Library Headers
#include <entt/entt.hpp>
#include <physx/PxPhysicsAPI.h>

#define MAX_DIRECTIONAL_LIGHTS 10
#define MAX_POINT_LIGHTS 1024
#define MAX_SPOT_LIGHTS 1024

using namespace physx;

namespace Louron {

	// Forward Declarations
	class Entity;
	class Prefab;
	class RenderPipeline;
	class UUID;

	struct ResourceManager;

	enum L_RENDER_PIPELINE;

	struct SceneConfig {

		std::string Name = "Untitled Scene";
		std::filesystem::path AssetDirectory;

		std::shared_ptr<RenderPipeline> ScenePipeline;
		std::shared_ptr<ResourceManager> SceneResourceManager;

		L_RENDER_PIPELINE ScenePipelineType;
	};

	class Scene : public Asset {

	public:

		Scene();
		Scene(L_RENDER_PIPELINE pipeline);
		~Scene();

		bool LoadSceneFile(const std::filesystem::path& sceneFilePath);

	public:

		Entity CreateEntity(const std::string& name = "");
		Entity CreateEntity(UUID uuid, const std::string& name = "");

		Entity InstantiatePrefab(std::shared_ptr<Prefab> prefab, std::optional<Transform> transform = std::nullopt, const UUID& parent_uuid = NULL_UUID);

		Entity DuplicateEntity(Entity entity);
		void DestroyEntity(Entity entity);
		
		Entity FindEntityByName(std::string_view name);
		Entity FindEntityByUUID(UUID uuid);

		bool HasEntity(const std::string& name);
		bool HasEntity(const UUID& uuid);

		bool IsRunning() const { return m_IsRunning; }
		bool IsPaused() const { return m_IsPaused; }
		bool IsPhysicsSimulating() const { return m_IsSimulatingPhysics; }

		void OnStart();
		void OnStop();

		void OnUpdate();
		void OnUpdateGUI();

		void OnFixedUpdate();
		void OnFixedUpdateGUI();

		template<typename... Components>
		auto GetAllEntitiesWith() {	return m_Registry.view<Components...>(); }

		Entity GetPrimaryCameraEntity();

		void SetConfig(const SceneConfig& config) { m_SceneConfig = config; }
		const SceneConfig& GetConfig() const { return m_SceneConfig; }
		std::shared_ptr<ResourceManager> GetResources() const { return m_SceneConfig.SceneResourceManager; }

		entt::registry* GetRegistry() { return &m_Registry; }
		bool CopyRegistry(std::shared_ptr<Scene> otherScene);

		PxScene* GetPhysScene() const { return m_PhysxScene; }
		void SetPhysScene(PxScene* physScene);

		virtual AssetType GetType() const override { return AssetType::Scene; }

	protected:

		entt::registry m_Registry;
		std::shared_ptr<std::unordered_map<UUID, entt::entity>> m_EntityMap = std::make_shared<std::unordered_map<UUID, entt::entity>>();

	private:

		PxScene* m_PhysxScene = nullptr;
		std::unique_ptr<CollisionCallback> m_CollisionCallback;

		bool m_IsRunning = false;
		bool m_IsPaused = false;
		bool m_IsSimulatingPhysics = false;

		std::filesystem::path m_SceneFilePath;
		SceneConfig m_SceneConfig;

		friend class Entity;
		friend class Project;
		friend class SceneSerializer;

		friend class TransformSystem;
		friend class PhysicsSystem;
	};
}