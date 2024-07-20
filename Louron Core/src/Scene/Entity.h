#pragma once

// Louron Core Headers
#include "Scene.h"

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

	//Base Entity Class
	class Entity {

	public:
		Entity() = default;
		Entity(entt::entity regHandle, Scene* scene);
		Entity(const Entity&) = default;


		// This adds a Component to the applicable Entity, and returns that Component
		template<typename T, typename... Args>
		T& AddComponent(Args&&... args) {

			#pragma region Rigidbody
			// Check if Component is Rigidbody Type
			if constexpr (std::is_same_v<T, Rigidbody>) {

				if (HasComponent<Rigidbody>())
					return GetComponent<Rigidbody>();
				
				Rigidbody& component = PhysicsSystem::AddRigidBody(*this, m_Scene);
				component.entity = std::make_shared<Entity>(*this);

				return component;
			}

			#pragma endregion

			#pragma region SphereCollider

			if constexpr (std::is_same_v<T, SphereCollider>) {

				if (HasComponent<SphereCollider>())
					return GetComponent<SphereCollider>();

				SphereCollider& component = PhysicsSystem::AddSphereCollider(*this, m_Scene);
				component.entity = std::make_shared<Entity>(*this);

				return component;
			}

			#pragma endregion

			#pragma region BoxCollider

			if constexpr (std::is_same_v<T, BoxCollider>) {

				if (HasComponent<BoxCollider>())
					return GetComponent<BoxCollider>();

				BoxCollider& component = PhysicsSystem::AddBoxCollider({ m_EntityHandle, m_Scene }, m_Scene);
				component.entity = std::make_shared<Entity>(*this);

				return component;
			}

			#pragma endregion

			if (HasComponent<T>()) {
				L_CORE_WARN("Entity Already Has: {0}", typeid(T).name());
				return GetComponent<T>();
			}

			T& component = m_Scene->m_Registry.emplace<T>(m_EntityHandle, std::forward<Args>(args)...);
			component.entity = std::make_shared<Entity>(*this);

			return component;
		}

		template<typename T>
		T& GetComponent() {
						
			// Return Blank Component - I want to be able to just call GetComponent, and 
			// not have to worry about error handling when an entity does not have a
			// specified component, rather, it logs in the console the entity does not
			// have a component.

			static std::unordered_map<std::type_index, std::shared_ptr<T>> blankComponents;
			if (m_EntityHandle == entt::null) {
				L_CORE_ERROR("Entity Cannot GetComponent as Entity Handle is Null");

				if (blankComponents.find(typeid(T)) == blankComponents.end())
					blankComponents[typeid(T)] = std::make_shared<T>();

				return *blankComponents[typeid(T)];
			}
			
			if (!HasComponent<T>()) {
				L_CORE_ERROR("Entity Does Not Have Component");

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

		template<typename... Components>
		bool HasAnyComponent() {
			return m_Scene->m_Registry.any<Components...>(m_EntityHandle);
		}

		// This removes any Component within the applicable Entity
		template <typename T>
		void RemoveComponent() {

			if constexpr (std::is_same_v<T, Rigidbody>) {
				if(HasComponent<Rigidbody>())
					PhysicsSystem::RemoveRigidBody({ m_EntityHandle, m_Scene }, m_Scene);
			}
			if constexpr (std::is_same_v<T, SphereCollider>) {
				if(HasComponent<SphereCollider>())
					PhysicsSystem::RemoveCollider({ m_EntityHandle, m_Scene }, m_Scene, PxGeometryType::eSPHERE);
			}
			if constexpr (std::is_same_v<T, BoxCollider>) {
				if (HasComponent<BoxCollider>())
					PhysicsSystem::RemoveCollider({ m_EntityHandle, m_Scene }, m_Scene, PxGeometryType::eBOX);
			}

			m_Scene->m_Registry.remove_if_exists<T>(m_EntityHandle);
		}

		operator bool() const { return m_Scene ? m_Scene->GetRegistry()->valid(m_EntityHandle) : m_EntityHandle != entt::null; }
		operator entt::entity() const { return m_EntityHandle; }
		operator uint32_t() const { return (uint32_t)m_EntityHandle; }

		// This returns the UUID reference to the Component
		UUID GetUUID() { return GetComponent<IDComponent>().ID; }
		// This returns the tag reference to the Component
		const std::string& GetName() { return GetComponent<TagComponent>().Tag; }

		Scene* GetScene() const { return m_Scene; }

	private:

		entt::entity m_EntityHandle{ entt::null };
		Scene* m_Scene = nullptr;
	};


}