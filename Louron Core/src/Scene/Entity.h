#pragma once

// Louron Core Headers
#include "Scene.h"

#include "../Asset/Asset Manager API.h"

#include "Components/Light.h"
#include "Components/UUID.h"
#include "Components/Mesh.h"
#include "Components/Components.h"
#include "Components/Physics/Collider.h"
#include "Components/Physics/Rigidbody.h"

#include "Scene Systems/Physics System.h"

#include "../Scripting/Script Manager.h"

// C++ Standard Library Headers
#include <iostream>
#include <typeindex>
#include <memory>
#include <utility>

// External Vendor Library Headers
#include <entt/entt.hpp>
#include <glm/glm.hpp>

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

			if (HasComponent<T>()) {
				L_CORE_WARN("Entity Already Has: {0}", typeid(T).name());
				return GetComponent<T>();
			}

			T& component = m_Scene->m_Registry.emplace<T>(m_EntityHandle, std::forward<Args>(args)...);

			component.scene = m_Scene;
			component.entity_uuid = m_Scene->m_Registry.get<IDComponent>(m_EntityHandle).ID;

			if constexpr (std::is_same_v<T, ScriptComponent>) {
				if(m_Scene && m_Scene->IsRunning())
					ScriptManager::OnCreateEntity(*this);
			}

			if constexpr (std::is_same_v<T, RigidbodyComponent>) {
				if ((m_Scene->IsRunning() || m_Scene->IsSimulating()) && m_Scene->GetPhysScene())
					component.Init(&GetComponent<TransformComponent>(), m_Scene->GetPhysScene());

				if (HasComponent<BoxColliderComponent>())
				{
					GetComponent<BoxColliderComponent>().AddFlag(ColliderFlag_RigidbodyUpdated);
					GetComponent<BoxColliderComponent>().AddFlag(ColliderFlag_ShapePropsUpdated);
					GetComponent<BoxColliderComponent>().AddFlag(ColliderFlag_TransformUpdated);
				}

				if (HasComponent<SphereColliderComponent>())
				{
					GetComponent<SphereColliderComponent>().AddFlag(ColliderFlag_RigidbodyUpdated);
					GetComponent<SphereColliderComponent>().AddFlag(ColliderFlag_ShapePropsUpdated);
					GetComponent<SphereColliderComponent>().AddFlag(ColliderFlag_TransformUpdated);
				}
			}

			if constexpr (std::is_same_v<T, SphereColliderComponent>) {
				if ((m_Scene->IsRunning() || m_Scene->IsSimulating()) && m_Scene->GetPhysScene())
					component.Init();

				if (HasComponent<RigidbodyComponent>())
				{
					if (auto actor_ref = GetComponent<RigidbodyComponent>().GetActor(); actor_ref && *actor_ref)
					{
						actor_ref->AddFlag(RigidbodyFlag_ShapesUpdated);
						actor_ref->AddFlag(RigidbodyFlag_TransformUpdated);
					}
				}

			}

			if constexpr (std::is_same_v<T, BoxColliderComponent>)
			{
				if ((m_Scene->IsRunning() || m_Scene->IsSimulating()) && m_Scene->GetPhysScene())
					component.Init();

				if (HasComponent<RigidbodyComponent>())
				{
					if (auto actor_ref = GetComponent<RigidbodyComponent>().GetActor(); actor_ref && *actor_ref)
					{
						actor_ref->AddFlag(RigidbodyFlag_ShapesUpdated);
						actor_ref->AddFlag(RigidbodyFlag_TransformUpdated);
					}
				}

				// Get the Optimal Size and Center for Box Collider Based on MeshFilter
				if (HasComponent<AssetMeshFilter>())
				{
					glm::mat4 global_transform = GetComponent<TransformComponent>().GetGlobalTransform();
					Bounds_AABB mesh_bounds = AssetManager::GetAsset<AssetMesh>(GetComponent<AssetMeshFilter>().MeshFilterAssetHandle)->MeshBounds;

					// Create OBB transformation matrix
					glm::mat4 obb_transform = glm::mat4(1.0f);
					obb_transform = glm::translate(obb_transform, mesh_bounds.Center());
					obb_transform = glm::scale(obb_transform, mesh_bounds.Size());

					// Apply OBB to Global Transform
					global_transform *= obb_transform;

					// Inverse Global so Center and Size are local
					glm::mat4 inv_global = glm::inverse(global_transform);

					// Extract Center
					glm::vec3 center = (inv_global * global_transform)[3];

					// Extract Local Half Extents
					glm::vec3 right = glm::vec3(inv_global * global_transform[0]);		// X basis vector
					glm::vec3 up = glm::vec3(inv_global * global_transform[1]);			// Y basis vector
					glm::vec3 forward = glm::vec3(inv_global * global_transform[2]);	// Z basis vector
					glm::vec3 size = glm::vec3(glm::length(right), glm::length(up), glm::length(forward)) * 0.5f;

					component.SetCentre(center);
					component.SetSize(size);
				}
			}

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

			if constexpr (std::is_same_v<T, RigidbodyComponent>) {
				if(HasComponent<RigidbodyComponent>())
					PhysicsSystem::RemoveRigidBody({ m_EntityHandle, m_Scene }, m_Scene);
			}
			if constexpr (std::is_same_v<T, SphereColliderComponent>) {
				if(HasComponent<SphereColliderComponent>())
					PhysicsSystem::RemoveCollider({ m_EntityHandle, m_Scene }, m_Scene, PxGeometryType::eSPHERE);
			}
			if constexpr (std::is_same_v<T, BoxColliderComponent>) {
				if (HasComponent<BoxColliderComponent>())
					PhysicsSystem::RemoveCollider({ m_EntityHandle, m_Scene}, m_Scene, PxGeometryType::eBOX);
			}

			m_Scene->m_Registry.remove_if_exists<T>(m_EntityHandle);
		}

		operator bool() const { return GetScene() ? GetScene()->GetRegistry()->valid(m_EntityHandle) : m_EntityHandle != entt::null; }
		operator entt::entity() const { return m_EntityHandle; }
		operator uint32_t() const { return (uint32_t)m_EntityHandle; }

		bool operator==(const Entity& other) const {
			return m_EntityHandle == other.m_EntityHandle && m_Scene == other.m_Scene;
		}

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