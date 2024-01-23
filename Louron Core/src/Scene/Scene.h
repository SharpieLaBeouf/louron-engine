#pragma once

#include <vector>
#include <memory>

#include "Resource Manager.h"

#include <entt/entt.hpp>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>


#define MAX_DIRECTIONAL_LIGHTS 10
#define MAX_POINT_LIGHTS 1024
#define MAX_SPOT_LIGHTS 1024

namespace Louron {

	class Entity;

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
		void OnUpdateGUI();
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