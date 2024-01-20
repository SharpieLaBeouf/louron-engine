#pragma once

#include <iostream>

#include "Scene.h"
#include "Components.h"
#include "Mesh.h"

#include "entt/entt.hpp"

namespace Louron {

	//Base Entity Class
	class Entity {

	public:
		Entity() = default;
		Entity(entt::entity regHandle, Scene* scene);
		Entity(const Entity&) = default;

		// This adds a Component to the applicable Entity, and returns that Component
		template<typename T, typename... Args>
		T& AddComponent(Args&&... args) {

			if (HasComponent<T>())
				return GetComponent<T>();

			T& component = m_Scene->m_Registry.emplace<T>(m_EntityHandle, std::forward<Args>(args)...);

			return component;
		}

		// This returns the applicable Component
		template<typename T>
		T& GetComponent() {
			if (m_EntityHandle == entt::null) {
				std::cerr << "[L20] ERROR: Attempted to Retrieve Component from NULL Entity!" << std::endl;
				assert(false);
			}
			
			return m_Scene->m_Registry.get<T>(m_EntityHandle);
		}

		// This returns if the Entity has an applicable Component
		template <typename T>
		bool HasComponent() {
			return m_Scene->m_Registry.has<T>(m_EntityHandle);
		}
		// This removes any Component within the applicable Entity
		template <typename T>
		void RemoveComponent() {
			m_Scene->m_Registry.remove_if_exists<T>(m_EntityHandle);
		}

		operator bool() const { return m_EntityHandle != entt::null; }
		operator entt::entity() const { return m_EntityHandle; }
		operator uint32_t() const { return (uint32_t)m_EntityHandle; }

		// This returns the tag reference to the Component
		const std::string& GetName() { return GetComponent<TagComponent>().Tag; }

	private:
		entt::entity m_EntityHandle{ entt::null };
		Scene* m_Scene = nullptr;
	};

}