#pragma once

// Louron Core Headers
#include "Scene.h"

#include "Components/UUID.h"
#include "Components/Mesh.h"
#include "Components/Components.h"

// C++ Standard Library Headers
#include <iostream>
#include <typeindex>
#include <memory>

// External Vendor Library Headers
#include <entt/entt.hpp>

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

		/// <summary>
		/// I want to be able to just call GetComponent, and not have to worry about 
		/// application breaking error handling when an entity does not have a
		/// specified component. Why do we want to halt execution when we should
		/// just log and debug this.
		/// </summary>
		template<typename T>
		T& GetComponent() {
						
			// Return Blank Component - I want to be able to just call GetComponent, and 
			// not have to worry about error handling when an entity does not have a
			// specified component, rather, it logs in the console the entity does not
			// have a component.

			static std::unordered_map<std::type_index, std::shared_ptr<T>> blankComponents;
			if (m_EntityHandle == entt::null) {
				std::cerr << "[L20] ERROR: Entity Cannot GetComponent as Entity Handle is Null!" << std::endl;

				if (blankComponents.find(typeid(T)) == blankComponents.end())
					blankComponents[typeid(T)] = std::make_shared<T>();

				return *blankComponents[typeid(T)];
			}
			
			if (!HasComponent<T>()) {
				std::cerr << "[L20] ERROR: Entity Does Not Have Component! " << m_Scene->m_Registry.get<TagComponent>(m_EntityHandle).Tag << " - " << typeid(T).name() << std::endl;


				if (blankComponents.find(typeid(T)) == blankComponents.end())
					blankComponents[typeid(T)] = std::make_shared<T>();

				return *blankComponents[typeid(T)];
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

		// This returns the UUID reference to the Component
		UUID GetUUID() { return GetComponent<IDComponent>().ID; }
		// This returns the tag reference to the Component
		const std::string& GetName() { return GetComponent<TagComponent>().Tag; }

	private:
		entt::entity m_EntityHandle{ entt::null };
		Scene* m_Scene = nullptr;
	};

}