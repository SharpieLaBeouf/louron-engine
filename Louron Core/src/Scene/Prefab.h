#pragma once

// Louron Core Headers
#include "../Asset/Asset.h"

#include "Components/UUID.h"
#include "Components/Mesh.h"
#include "Components/Components.h"
#include "Components/Physics/Collider.h"
#include "Components/Physics/Rigidbody.h"

#include "Scene Systems/Physics System.h"

// C++ Standard Library Headers
#include <iostream>
#include <typeindex>
#include <memory>
#include <utility>

// External Vendor Library Headers
#include <entt/entt.hpp>

namespace Louron {

	class Entity;
		
	class Prefab : public Asset {

	public:

		Prefab();
		Prefab(const Prefab&) = delete;
		~Prefab() = default;

		/// <summary>
		/// Use this function to copy the data structure of an existing Entity into a Prefab Asset.
		/// </summary>
		Prefab(Entity scene_entity);

		/// <summary>
		/// Create an entity within the Prefab
		/// </summary>
		entt::entity CreateEntity(const std::string& name = "");

		/// <summary>
		/// Destroy the entity within the Prefab
		/// </summary>
		/// <param name="entity"></param>
		void DestroyEntity(entt::entity entity);

		entt::entity FindEntityByName(const std::string& name);
		entt::entity FindEntityByUUID(const UUID& uuid);

		bool HasEntity(const std::string& name);

		template<typename... Components>
		auto GetAllEntitiesWith() { return m_PrefabRegistry.view<Components...>(); }

		entt::registry* GetRegistry() { return &m_PrefabRegistry; }

		void Serialize();
		bool Deserialize();

		virtual AssetType GetType() const override { return AssetType::Prefab; }

		operator bool() const { return m_RootEntity != entt::null; }

	private:

		std::unordered_map<UUID, entt::entity> m_EntityMap;

		entt::entity CopyEntity(Entity start_entity, UUID parent_uuid);

		entt::registry m_PrefabRegistry;
		entt::entity m_RootEntity = entt::null;

		std::string m_PrefabName;

		int m_InstantiationCount = 0;
		
		friend class Scene;
	};

}