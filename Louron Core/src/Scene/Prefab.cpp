#include "Prefab.h"

#include "Entity.h"

#include "Components/Components.h"
#include "Components/Light.h"
#include "Components/Mesh.h"
#include "Components/UUID.h"
#include "Components/Skybox.h"

#include "Components/Physics/Collider.h"
#include "Components/Physics/Rigidbody.h"
#include "Components/Physics/PhysicsWrappers.h"

#ifndef YAML_CPP_STATIC_DEFINE
#define YAML_CPP_STATIC_DEFINE
#endif
#include <yaml-cpp/yaml.h>

namespace YAML {

	template<>
	struct convert<glm::vec2>
	{
		static Node encode(const glm::vec2& rhs)
		{
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			node.SetStyle(EmitterStyle::Flow);
			return node;
		}

		static bool decode(const Node& node, glm::vec2& rhs)
		{
			if (!node.IsSequence() || node.size() != 2)
				return false;

			rhs.x = node[0].as<float>();
			rhs.y = node[1].as<float>();
			return true;
		}
	};

	template<>
	struct convert<glm::vec3>
	{
		static Node encode(const glm::vec3& rhs)
		{
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			node.push_back(rhs.z);
			node.SetStyle(EmitterStyle::Flow);
			return node;
		}

		static bool decode(const Node& node, glm::vec3& rhs)
		{
			if (!node.IsSequence() || node.size() != 3)
				return false;

			rhs.x = node[0].as<float>();
			rhs.y = node[1].as<float>();
			rhs.z = node[2].as<float>();
			return true;
		}
	};

	template<>
	struct convert<glm::vec4>
	{
		static Node encode(const glm::vec4& rhs)
		{
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			node.push_back(rhs.z);
			node.push_back(rhs.w);
			node.SetStyle(EmitterStyle::Flow);
			return node;
		}

		static bool decode(const Node& node, glm::vec4& rhs)
		{
			if (!node.IsSequence() || node.size() != 4)
				return false;

			rhs.x = node[0].as<float>();
			rhs.y = node[1].as<float>();
			rhs.z = node[2].as<float>();
			rhs.w = node[3].as<float>();
			return true;
		}
	};

	template<>
	struct convert<Louron::UUID>
	{
		static Node encode(const Louron::UUID& uuid)
		{
			Node node;
			node.push_back((uint32_t)uuid);
			return node;
		}

		static bool decode(const Node& node, Louron::UUID& uuid)
		{
			uuid = node.as<uint32_t>();
			return true;
		}
	};
}

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
		AddComponent<IDComponent>(entity, (uint32_t)entity);

		// 2. Add Transform Component
		AddComponent<TransformComponent>(entity);

		// 3. Add Tag Component
		auto& tag = AddComponent<TagComponent>(entity);
		tag.Tag = name.empty() ? "Untitled Entity" : name;

		// 4. Add Hierarchy Component
		auto& hierarchy = AddComponent<HierarchyComponent>(entity);
		if(m_PrefabRegistry.valid(m_RootEntity) && m_PrefabRegistry.has<IDComponent>(m_RootEntity)) {
			hierarchy.m_Parent = m_PrefabRegistry.get<IDComponent>(m_RootEntity).ID;
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
			if (start_entity.HasComponent<TransformComponent>()) {
				auto& component = start_entity.GetComponent<TransformComponent>();
				m_PrefabRegistry.emplace_or_replace<TransformComponent>(prefab_entity_handle, component);
			}

			// 1.g. MeshFilter
			if (start_entity.HasComponent<AssetMeshFilter>()) {
				auto& component = start_entity.GetComponent<AssetMeshFilter>();
				m_PrefabRegistry.emplace_or_replace<AssetMeshFilter>(prefab_entity_handle, component);
			}

			// 1.h. MeshRenderer
			if (start_entity.HasComponent<AssetMeshRenderer>()) {
				auto& component = start_entity.GetComponent<AssetMeshRenderer>();
				m_PrefabRegistry.emplace_or_replace<AssetMeshRenderer>(prefab_entity_handle, component);
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
			if (start_entity.HasComponent<RigidbodyComponent>()) {
				auto& component = start_entity.GetComponent<RigidbodyComponent>();
				m_PrefabRegistry.emplace_or_replace<RigidbodyComponent>(prefab_entity_handle, component);
			}

			// 1.n. Sphere Collider
			if (start_entity.HasComponent<SphereColliderComponent>()) {
				auto& component = start_entity.GetComponent<SphereColliderComponent>();
				m_PrefabRegistry.emplace_or_replace<SphereColliderComponent>(prefab_entity_handle, component);
			}

			// 1.o. Box Collider
			if (start_entity.HasComponent<BoxColliderComponent>()) {
				auto& component = start_entity.GetComponent<BoxColliderComponent>();
				m_PrefabRegistry.emplace_or_replace<BoxColliderComponent>(prefab_entity_handle, component);
			}

			// 1.o. Script Component
			if (start_entity.HasComponent<ScriptComponent>()) {
				auto& component = start_entity.GetComponent<ScriptComponent>();
				m_PrefabRegistry.emplace_or_replace<ScriptComponent>(prefab_entity_handle, component);
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

	void Prefab::SerializeSubEntity(YAML::Emitter& out, entt::entity entity)
	{

		out << YAML::BeginMap;

		if (HasComponent<IDComponent>(entity)) {
			GetComponent<IDComponent>(entity).Serialize(out);
		}

		if (HasComponent<TagComponent>(entity)) {
			GetComponent<TagComponent>(entity).Serialize(out);
		}

		if (HasComponent<HierarchyComponent>(entity)) {
			GetComponent<HierarchyComponent>(entity).Serialize(out);
		}

		if (HasComponent<ScriptComponent>(entity)) {
			GetComponent<ScriptComponent>(entity).Serialize(out);
		}

		if (HasComponent<TransformComponent>(entity)) {
			GetComponent<TransformComponent>(entity).Serialize(out);
		}

		if (HasComponent<CameraComponent>(entity)) {
			GetComponent<CameraComponent>(entity).Serialize(out);
		}

		if (HasComponent<AssetMeshFilter>(entity)) {
			GetComponent<AssetMeshFilter>(entity).Serialize(out);
		}

		if (HasComponent<AssetMeshRenderer>(entity)) {
			GetComponent<AssetMeshRenderer>(entity).Serialize(out);
		}

		if (HasComponent<SkyboxComponent>(entity)) {
			GetComponent<SkyboxComponent>(entity).Serialize(out);
		}

		if (HasComponent<PointLightComponent>(entity)) {
			GetComponent<PointLightComponent>(entity).Serialize(out);
		}

		if (HasComponent<SpotLightComponent>(entity)) {
			GetComponent<SpotLightComponent>(entity).Serialize(out);
		}

		if (HasComponent<DirectionalLightComponent>(entity)) {
			GetComponent<DirectionalLightComponent>(entity).Serialize(out);
		}

		if (HasComponent<RigidbodyComponent>(entity)) {
			GetComponent<RigidbodyComponent>(entity).Serialize(out);
		}

		if (HasComponent<SphereColliderComponent>(entity)) {
			GetComponent<SphereColliderComponent>(entity).Serialize(out);
		}

		if (HasComponent<BoxColliderComponent>(entity)) {
			GetComponent<BoxColliderComponent>(entity).Serialize(out);
		}

		out << YAML::EndMap;

	}

	void Prefab::DeserializeSubEntity(entt::entity entity, entt::entity parent_entity, const std::unordered_map<UUID, YAML::Node>& entity_node_map, UUID node_index)
	{
		const YAML::Node entity_node = entity_node_map.at(node_index);

		// UUID
		UUID uuid = entity_node["Entity"].as<uint32_t>();
		std::string tag_string = entity_node["TagComponent"]["Tag"].as<std::string>();
		GetComponent<TagComponent>(entity).Deserialize(entity_node["TagComponent"]);

		// Hierarchy
		auto hierarchy = entity_node["HierarchyComponent"];
		if (hierarchy) {
			auto& entityHierarchy = GetComponent<HierarchyComponent>(entity);
			entityHierarchy.Deserialize(hierarchy);
		}

		// Script
		auto script = entity_node["ScriptComponent"];
		if (script) {
			auto& entityScript = AddComponent<ScriptComponent>(entity);
			entityScript.Deserialize(script, uuid);
		}

		// Transform
		auto transform = entity_node["TransformComponent"];
		if (transform) {
			auto& entityTransform = GetComponent<TransformComponent>(entity);
			entityTransform.Deserialize(transform);
		}

		// Camera
		auto camera = entity_node["CameraComponent"];
		if (camera) {

			auto& entityCamera = AddComponent<CameraComponent>(entity);
			entityCamera.CameraInstance = std::make_shared<SceneCamera>();

			if (!entityCamera.Deserialize(camera))
				L_CORE_WARN("Deserialisation of Camera Component Not Complete.");
		}

		// Mesh Filter
		auto meshFilter = entity_node["MeshFilterComponent"];
		if (meshFilter) {

			auto& entityMeshFilter = AddComponent<AssetMeshFilter>(entity);

			if (!entityMeshFilter.Deserialize(meshFilter))
				L_CORE_WARN("Deserialisation of Mesh Filter Not Complete.");
		}

		// Mesh Renderer
		auto meshRenderer = entity_node["MeshRendererComponent"];
		if (meshRenderer) {

			auto& entityMeshRenderer = AddComponent<AssetMeshRenderer>(entity);

			if (!entityMeshRenderer.Deserialize(meshRenderer))
				L_CORE_WARN("Deserialisation of Mesh Renderer Not Complete.");
		}

		// Skybox Component and Skybox Material
		auto skybox = entity_node["SkyboxComponent"];
		if (skybox) {

			auto& skyboxComponent = AddComponent<SkyboxComponent>(entity);

			if (!skyboxComponent.Deserialize(skybox))
				L_CORE_WARN("Deserialisation of Sky Box Not Complete.");
		}

		// Point Light
		auto pointLight = entity_node["PointLightComponent"];
		if (pointLight) {

			auto& entityPointLight = AddComponent<PointLightComponent>(entity);

			if (!entityPointLight.Deserialize(pointLight))
				L_CORE_WARN("Deserialisation of Point Light Not Complete.");
		}

		// Spot Light
		auto spotLight = entity_node["SpotLightComponent"];
		if (spotLight) {

			auto& entitySpotLight = AddComponent<SpotLightComponent>(entity);

			if (!entitySpotLight.Deserialize(spotLight))
				L_CORE_WARN("Deserialisation of Spot Light Not Complete.");
		}

		// Directional Light
		auto directionalLight = entity_node["DirectionalLightComponent"];
		if (directionalLight) {

			auto& entityDirectionalLight = AddComponent<DirectionalLightComponent>(entity);

			if (!entityDirectionalLight.Deserialize(directionalLight))
				L_CORE_WARN("Deserialisation of Directional Light Not Complete.");
		}

		// Rigidbody
		auto rigidBody = entity_node["RigidbodyComponent"];
		if (rigidBody) {

			auto& entityRigidBody = AddComponent<RigidbodyComponent>(entity);

			if (!entityRigidBody.Deserialize(rigidBody))
				L_CORE_WARN("Deserialisation of Rigidbody Not Complete.");
		}

		// Sphere Collider
		auto sphereCollider = entity_node["SphereColliderComponent"];
		if (sphereCollider) {

			auto& entitySphereCollider = AddComponent<SphereColliderComponent>(entity);

			if (!entitySphereCollider.Deserialize(sphereCollider))
				L_CORE_WARN("Deserialisation of Sphere Collider Not Complete.");
		}

		// Box Collider
		auto boxCollider = entity_node["BoxColliderComponent"];
		if (boxCollider) {

			auto& entityBoxCollider = AddComponent<BoxColliderComponent>(entity);

			if (!entityBoxCollider.Deserialize(boxCollider))
				L_CORE_WARN("Deserialisation of Box Collider Not Complete.");
		}

		// Have to defer creating child entities to the end!
		// This is because when creating a new entity, ENTT changes all the data up boo!
		std::vector<UUID> children = GetComponent<HierarchyComponent>(entity).GetChildren(); // Make copy 
		for (auto& child : children) {
			entt::entity child_entity = CreateEntity("To Deserialise");
			DeserializeSubEntity(child_entity, entity, entity_node_map, child);
		}
		children = GetComponent<HierarchyComponent>(entity).GetChildren();
	}

	// TODO: IMPLEMENT
	bool Prefab::Serialize(const std::filesystem::path& file_path, const AssetMetaData& asset_meta_data) {

		// Is path a file or directory?
		if (std::filesystem::is_directory(file_path)) {
			L_CORE_ERROR("Could Not Serialise Prefab as Directory.");
			return false;
		}

		// Create Directory if Doesn't Exist
		if (!std::filesystem::exists(file_path.parent_path()))
			std::filesystem::create_directories(file_path.parent_path());

		if (file_path.extension() != ".lprefab") {

			L_CORE_WARN("Incompatible Prefab File Extension");
			L_CORE_WARN("	Extension Used: {0}", file_path.extension().string());
			L_CORE_WARN("	Extension Expected: ..lprefab");
			return false;
		}

		YAML::Emitter out;
		out << YAML::BeginMap;
		out << YAML::Key << "Prefab Name" << YAML::Value << m_PrefabName;
		out << YAML::Key << "PrefabEntities" << YAML::Value << YAML::BeginSeq;

		m_PrefabRegistry.each(
			[&](auto entityID){
				entt::entity entity = entityID;
				if (entity == entt::null)
					return;

				SerializeSubEntity(out, entity);
			}
		);

		out << YAML::EndSeq << YAML::EndMap;

		std::ofstream fout(file_path);
		fout << out.c_str();
		return true;
	}

	// TODO: IMPLEMENT
	bool Prefab::Deserialize(const std::filesystem::path& file_path) {

		if (file_path.extension() != ".lprefab") {

			L_CORE_WARN("Incompatible Prefab File Extension");
			L_CORE_WARN("	Extension Used: {0}", file_path.extension().string());
			L_CORE_WARN("	Extension Expected: ..lprefab");
			return false;
		}

		YAML::Node data;

		try {
			data = YAML::LoadFile(file_path.string());
		}
		catch (YAML::ParserException e) {
			L_CORE_ERROR("YAML-CPP Failed to Load Prefab File: '{0}', {1}", file_path.string(), e.what());
			return false;
		}

		if (!data["Prefab Name"]) {
			L_CORE_ERROR("Prefab Name Node Not Correctly Declared in File: \'{0}\'", file_path.string());
			return false;
		}
		else {
			m_PrefabName = data["Prefab Name"].as<std::string>();
		}

		YAML::Node entities = data["PrefabEntities"];

		if (!entities)
			return false; // No Data Node

		if (entities.size() < 1)
			return false; // No Actual Entities
		
		// We won't need this as we will get all ROOT entities and their children through the following
		//DestroyEntity(m_RootEntity);

		std::unordered_map<UUID, YAML::Node> node_map; // Store the node by value
		for (int i = 0; i < entities.size(); i++) {
			UUID uuid = entities[i]["Entity"].as<uint32_t>();
			node_map[uuid] = entities[i];
		}
		
		// Find All ROOT entities and create them (they will create their own children)
		for (auto entity : entities) {

			UUID uuid = entity["Entity"].as<uint32_t>();

			auto hierarchy_data = entity["HierarchyComponent"];
			if (hierarchy_data) {

				HierarchyComponent temp_hierarchy_component{};
				temp_hierarchy_component.Deserialize(hierarchy_data);

				if (!temp_hierarchy_component.HasParent()) {
					DeserializeSubEntity(m_RootEntity, entt::null, node_map, uuid);
				}
			}
		}

		return true;
	}

}