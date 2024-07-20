#include "Prefab.h"

#include "Entity.h"

#include "Components/Camera.h"
#include "Components/Components.h"
#include "Components/Light.h"
#include "Components/Mesh.h"
#include "Components/UUID.h"
#include "Components/Skybox.h"

#include "Components/Physics/Collider.h"
#include "Components/Physics/Rigidbody.h"
#include "Components/Physics/PhysicsWrappers.h"

namespace Louron {

	Prefab::Prefab() {

		m_RootEntity = CreateEntity("Prefab Root Entity");

	}

	Prefab::Prefab(Entity scene_entity) {

		if (!scene_entity) {
			L_CORE_ERROR("Cannot Create Prefab From Scene - Entity is Invalid.");
			return;
		}

		CopyEntity(scene_entity, NULL_UUID);

	}

	entt::entity Prefab::CreateEntity(const std::string& name) {
		entt::entity entity = m_PrefabRegistry.create();

		// 1. Add UUID Component
		// ID Component - We do not set this to the UUID of the 
		// Entity in the Scene - rather the Prefab's entt handle 
		// of the current entity. When this is instantiated back into
		// a scene, all entities and children UUID's are remapped
		// to ensure this prefab does not impact other entities
		// throughout the Scene that are already instantiated from
		// this prefab!
		m_PrefabRegistry.emplace_or_replace<IDComponent>(entity, (uint32_t)entity);

		// 2. Add Transform Component
		m_PrefabRegistry.emplace_or_replace<Transform>(entity);

		// 3. Add Tag Component
		auto& tag = m_PrefabRegistry.emplace_or_replace<TagComponent>(entity);
		tag.Tag = name.empty() ? "Untitled Entity" : name;

		// 4. Add Hierarchy Component
		auto& hierarchy = m_PrefabRegistry.emplace_or_replace<HierarchyComponent>(entity);
		if(m_PrefabRegistry.valid(m_RootEntity) && m_PrefabRegistry.has<IDComponent>(m_RootEntity)) {
			hierarchy.AttachParent(m_PrefabRegistry.get<IDComponent>(m_RootEntity).ID);
		}

		m_EntityMap.emplace((uint32_t)entity, entity);

		return entity;
	}

	void Louron::Prefab::DestroyEntity(entt::entity entity) {

		if (entity == entt::null) {
			L_CORE_WARN("Attempted to Destroy Null Prefab Entity.");
			return;
		}

		if (!m_PrefabRegistry.valid(entity)) {
			L_CORE_WARN("Attempted to Destroy an Entity Not In The Prefab.");
			return;
		}

		m_EntityMap.erase((uint32_t)entity);
		m_PrefabRegistry.destroy(entity);
		return;
	}

	entt::entity Prefab::FindEntityByName(const std::string& name)
	{
		auto view = m_PrefabRegistry.view<TagComponent>();
		for (auto entity : view) {
			const TagComponent& tag = view.get<TagComponent>(entity);
			if (tag.Tag == name)
				return entity;
		}

		L_CORE_ERROR("Prefab Does Not Have an Entity Named: {0}", name);
		return entt::null;
	}

	entt::entity Prefab::FindEntityByUUID(const UUID& uuid)
	{
		if (m_EntityMap.find(uuid) != m_EntityMap.end()) {
			return m_EntityMap.at(uuid);
		}

		L_CORE_ERROR("Entity UUID not found in scene");
		return entt::null;
	}

	bool Prefab::HasEntity(const std::string& name) {
		return m_PrefabRegistry.valid(FindEntityByName(name));
	}

	// TODO: IMPLEMENT
	void Prefab::Serialize() {
		return;
	}

	// TODO: IMPLEMENT
	bool Louron::Prefab::Deserialize() {
		return false;
	}

	entt::entity Prefab::CopyEntity(Entity start_entity, UUID parent_uuid) {

		entt::entity prefab_entity_handle = entt::null;

		if (parent_uuid == NULL_UUID) {
			prefab_entity_handle = CreateEntity("Prefab Root Entity");
			m_RootEntity = prefab_entity_handle;
		}
		else {
			prefab_entity_handle = CreateEntity();
		}

		// 1. Copy Data in All Components
		{
			// 1.a. Tag Component
			if (start_entity.HasComponent<TagComponent>()) {
				auto& component = start_entity.GetComponent<TagComponent>();
				auto& prefab_component = m_PrefabRegistry.emplace_or_replace<TagComponent>(prefab_entity_handle, component);
				if (parent_uuid == NULL_UUID)
					m_PrefabName = prefab_component.Tag;
			}

			// 1.b. Hierarchy Component
			if (start_entity.HasComponent<HierarchyComponent>()) {
				auto& component = start_entity.GetComponent<HierarchyComponent>();
				auto& prefab_component = m_PrefabRegistry.emplace_or_replace<HierarchyComponent>(prefab_entity_handle, component);
				prefab_component.m_Parent = parent_uuid;
			}

			// 1.c. Camera Component
			if (start_entity.HasComponent<CameraComponent>()) {
				auto& component = start_entity.GetComponent<CameraComponent>();
				m_PrefabRegistry.emplace_or_replace<CameraComponent>(prefab_entity_handle, component);
			}

			// 1.d. Audio Listener
			if (start_entity.HasComponent<AudioListener>()) {
				auto& component = start_entity.GetComponent<AudioListener>();
				m_PrefabRegistry.emplace_or_replace<AudioListener>(prefab_entity_handle, component);
			}

			// 1.e. Audio Emitter
			if (start_entity.HasComponent<AudioEmitter>()) {
				auto& component = start_entity.GetComponent<AudioEmitter>();
				m_PrefabRegistry.emplace_or_replace<AudioEmitter>(prefab_entity_handle, component);
			}

			// 1.f. Transform Component
			if (start_entity.HasComponent<Transform>()) {
				auto& component = start_entity.GetComponent<Transform>();
				m_PrefabRegistry.emplace_or_replace<Transform>(prefab_entity_handle, component);
			}

			// 1.g. MeshFilter
			if (start_entity.HasComponent<MeshFilter>()) {
				auto& component = start_entity.GetComponent<MeshFilter>();
				m_PrefabRegistry.emplace_or_replace<MeshFilter>(prefab_entity_handle, component);
			}

			// 1.h. MeshRenderer
			if (start_entity.HasComponent<MeshRenderer>()) {
				auto& component = start_entity.GetComponent<MeshRenderer>();
				m_PrefabRegistry.emplace_or_replace<MeshRenderer>(prefab_entity_handle, component);
			}

			// 1.i. PointLight Component
			if (start_entity.HasComponent<PointLightComponent>()) {
				auto& component = start_entity.GetComponent<PointLightComponent>();
				m_PrefabRegistry.emplace_or_replace<PointLightComponent>(prefab_entity_handle, component);
			}

			// 1.j. SpotLight Component
			if (start_entity.HasComponent<SpotLightComponent>()) {
				auto& component = start_entity.GetComponent<SpotLightComponent>();
				m_PrefabRegistry.emplace_or_replace<SpotLightComponent>(prefab_entity_handle, component);
			}

			// 1.k. DirectionalLight Component
			if (start_entity.HasComponent<DirectionalLightComponent>()) {
				auto& component = start_entity.GetComponent<DirectionalLightComponent>();
				m_PrefabRegistry.emplace_or_replace<DirectionalLightComponent>(prefab_entity_handle, component);
			}

			// 1.l. Skybox Component
			if (start_entity.HasComponent<SkyboxComponent>()) {
				auto& component = start_entity.GetComponent<SkyboxComponent>();
				m_PrefabRegistry.emplace_or_replace<SkyboxComponent>(prefab_entity_handle, component);
			}

			// 1.m. Rigidbody Component
			if (start_entity.HasComponent<Rigidbody>()) {
				auto& component = start_entity.GetComponent<Rigidbody>();
				m_PrefabRegistry.emplace_or_replace<Rigidbody>(prefab_entity_handle, component);
			}

			// 1.n. Sphere Collider
			if (start_entity.HasComponent<SphereCollider>()) {
				auto& component = start_entity.GetComponent<SphereCollider>();
				m_PrefabRegistry.emplace_or_replace<SphereCollider>(prefab_entity_handle, component);
			}

			// 1.o. Box Collider
			if (start_entity.HasComponent<BoxCollider>()) {
				auto& component = start_entity.GetComponent<BoxCollider>();
				m_PrefabRegistry.emplace_or_replace<BoxCollider>(prefab_entity_handle, component);
			}
		}

		// 2. Recurse Children
		if (start_entity.HasComponent<HierarchyComponent>()) {

			for (const auto& child_uuid : start_entity.GetComponent<HierarchyComponent>().GetChildren()) {

				entt::entity child_entity = CopyEntity(start_entity.GetScene()->FindEntityByUUID(child_uuid), (uint32_t)prefab_entity_handle);

				if (m_PrefabRegistry.has<HierarchyComponent>(prefab_entity_handle)) {
					m_PrefabRegistry.get<HierarchyComponent>(prefab_entity_handle).m_Children.push_back((uint32_t)child_entity);
				}
			}
		}

		return prefab_entity_handle;
	}
}