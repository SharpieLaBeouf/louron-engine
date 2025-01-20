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

namespace YAML { class Emitter; class Node; }

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

		template<typename T, typename... Args>
		T& AddComponent(entt::entity entity_handle, Args&&... args) {

			if (HasComponent<T>(entity_handle)) {
				L_CORE_WARN("Entity Already Has: {0}", typeid(T).name());
				return GetComponent<T>(entity_handle);
			}

			T& component = m_PrefabRegistry.emplace<T>(entity_handle, std::forward<Args>(args)...);

			return component;

		}

		template<typename T>
		T& GetComponent(entt::entity entity_handle) {

			static std::unordered_map<std::type_index, std::shared_ptr<T>> blankComponents;
			if (entity_handle == entt::null) {
				L_CORE_ERROR("Entity Cannot GetComponent as Entity Handle is Null");

				if (blankComponents.find(typeid(T)) == blankComponents.end())
					blankComponents[typeid(T)] = std::make_shared<T>();

				return *blankComponents[typeid(T)];
			}

			if (!HasComponent<T>(entity_handle)) {
				L_CORE_ERROR("Entity Does Not Have Component");

				if (blankComponents.find(typeid(T)) == blankComponents.end())
					blankComponents[typeid(T)] = std::make_shared<T>();

				return *blankComponents[typeid(T)];
			}

			return m_PrefabRegistry.get<T>(entity_handle);

		}

		template <typename T>
		void RemoveComponent(entt::entity entity_handle) {

			m_PrefabRegistry.remove_if_exists<T>(entity_handle);
		}

		// This returns if the Entity has an applicable Component
		template <typename T>
		bool HasComponent(entt::entity entity) {
			return m_PrefabRegistry.has<T>(entity);
		}

		const std::string& GetPrefabName() const { return m_PrefabName; }
		void SetPrefabName(const std::string& name) { 
			m_PrefabName = name;
			if (m_RootEntity != entt::null) {
				GetComponent<TagComponent>(m_RootEntity).Tag = name;
			}
		}

		const entt::entity& GetRootEntity() const { return m_RootEntity; }

		entt::entity FindEntityByName(const std::string& name);
		entt::entity FindEntityByUUID(const UUID& uuid);

		bool HasEntity(const std::string& name);

		template<typename... Components>
		auto GetAllEntitiesWith() { return m_PrefabRegistry.view<Components...>(); }

		entt::registry* GetRegistry() { return &m_PrefabRegistry; }
		
		/// <summary>
		/// Will Serialise the Prefab into a file.
		/// </summary>
		/// <param name="file_path">Needs to be valid file path with .lprefab extension.</param>
		bool Serialize(const std::filesystem::path& file_path, const AssetMetaData& asset_meta_data);
		bool Deserialize(const std::filesystem::path& file_path);

		virtual AssetType GetType() const override { return AssetType::Prefab; }

		operator bool() const { return m_RootEntity != entt::null; }

		bool IsMutable() const { return m_Mutable; }
		void SetMutable(const bool& is_mutable) { m_Mutable = is_mutable; }

	private:

		void SerializeSubEntity(YAML::Emitter& out, entt::entity entity);
		void DeserializeSubEntity(entt::entity entity, entt::entity parent_entity, const std::unordered_map<UUID, YAML::Node>& entity_node_map, UUID node_index);


		std::unordered_map<UUID, entt::entity> m_EntityMap;

		entt::entity CopyEntity(Entity start_entity, UUID parent_uuid);

		entt::registry m_PrefabRegistry;
		entt::entity m_RootEntity = entt::null;

		std::string m_PrefabName;

		bool m_Mutable = false;
		
		friend class Scene;
	};

}