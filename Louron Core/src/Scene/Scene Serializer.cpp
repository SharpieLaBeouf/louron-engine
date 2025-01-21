#include "Scene Serializer.h"

// Louron Core Headers
#include "Scene.h"
#include "Entity.h"
#include "OctreeBounds.h"

#include "Components/Components.h"
#include "Components/Light.h"
#include "Components/Skybox.h"
#include "Components/UUID.h"

#include "Components/Physics/Collider.h"
#include "Components/Physics/Rigidbody.h"
#include "Components/Physics/PhysicsWrappers.h"

#include "Scene Systems/Physics System.h"

#include "../Renderer/RendererPipeline.h"

#include "../Core/Time.h"

// C++ Standard Library Headers
#include <fstream>

// External Vendor Library Headers
#include <glm/gtx/string_cast.hpp>

#pragma warning( push )
#pragma warning( disable : 4099)

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
	
	SceneSerializer::SceneSerializer(std::shared_ptr<Scene> scene) : m_Scene(scene) { }
	
	void SceneSerializer::SerializeEntity(YAML::Emitter& out, Entity entity) {

		out << YAML::BeginMap;

		if (entity.HasComponent<IDComponent>()) {
			entity.GetComponent<IDComponent>().Serialize(out);
		}

		if (entity.HasComponent<TagComponent>()) {
			entity.GetComponent<TagComponent>().Serialize(out);
		}

		if (entity.HasComponent<HierarchyComponent>()) {
			entity.GetComponent<HierarchyComponent>().Serialize(out);
		}

		if (entity.HasComponent<ScriptComponent>()) {
			entity.GetComponent<ScriptComponent>().Serialize(out);
		}

		if (entity.HasComponent<TransformComponent>()) {
			entity.GetComponent<TransformComponent>().Serialize(out);
		}

		if (entity.HasComponent<CameraComponent>()) {
			entity.GetComponent<CameraComponent>().Serialize(out);
		}

		if (entity.HasComponent<AssetMeshFilter>()) {
			entity.GetComponent<AssetMeshFilter>().Serialize(out);
		}

		if (entity.HasComponent<AssetMeshRenderer>()) {
			entity.GetComponent<AssetMeshRenderer>().Serialize(out);
		}

		if (entity.HasComponent<SkyboxComponent>()) {
			entity.GetComponent<SkyboxComponent>().Serialize(out);
		}

		if (entity.HasComponent<PointLightComponent>()) {
			entity.GetComponent<PointLightComponent>().Serialize(out);
		}
		
		if (entity.HasComponent<SpotLightComponent>()) {
			entity.GetComponent<SpotLightComponent>().Serialize(out);
		}
		
		if (entity.HasComponent<DirectionalLightComponent>()) {
			entity.GetComponent<DirectionalLightComponent>().Serialize(out);
		}

		if (entity.HasComponent<RigidbodyComponent>()) {
			entity.GetComponent<RigidbodyComponent>().Serialize(out);
		}

		if (entity.HasComponent<SphereColliderComponent>()) {
			entity.GetComponent<SphereColliderComponent>().Serialize(out);
		}

		if (entity.HasComponent<BoxColliderComponent>()) {
			entity.GetComponent<BoxColliderComponent>().Serialize(out);
		}

		out << YAML::EndMap;
	}

	bool SceneSerializer::Serialize(const std::filesystem::path& sceneFilePath) {

		auto scene_ref = m_Scene.lock();
		if (!scene_ref) {
			L_CORE_ERROR("Scene Invalid - Cannot Serialize.");
			return false;
		}

		std::filesystem::path outFilePath = (sceneFilePath.empty()) ? scene_ref->m_SceneConfig.SceneFilePath : sceneFilePath;

		if (outFilePath.extension() != ".lscene") {

			L_CORE_WARN("Incompatible Scene File Extension");
			L_CORE_WARN("Extension Used: {0}", outFilePath.extension().string());
			L_CORE_WARN("Extension Expected: .lscene");
			return false;
		}

		YAML::Emitter out;
		out << YAML::BeginMap;
		out << YAML::Key << "Scene Name" << YAML::Value << scene_ref->m_SceneConfig.Name;
		out << YAML::Key << "Scene Asset Directory" << YAML::Value << scene_ref->m_SceneConfig.AssetDirectory.string();
		out << YAML::Key << "Scene Pipeline Type" << YAML::Value << std::to_string((uint8_t)scene_ref->m_SceneConfig.ScenePipelineType);

		out << YAML::Key << "Entities" << YAML::Value << YAML::BeginSeq;

		scene_ref->m_Registry.each([&](auto entityID)
			{
				Entity entity = { entityID, scene_ref.get()};
				if (!entity)
					return;

				SerializeEntity(out, entity);
			});

		out << YAML::EndSeq << YAML::EndMap;

		if(outFilePath.has_parent_path())
			std::filesystem::create_directories(outFilePath.parent_path());

		std::ofstream fout(outFilePath);
		fout << out.c_str();
		return true;
		
	}

	bool SceneSerializer::Deserialize(const std::filesystem::path& sceneFilePath) {

		auto scene_ref = m_Scene.lock();
		if (!scene_ref) {
			L_CORE_ERROR("Scene Invalid - Cannot Deserialize.");
			return false;
		}

		if (sceneFilePath.extension() != ".lscene") {

			L_CORE_WARN("Incompatible Scene File Extension");
			L_CORE_WARN("Extension Used: {0}", sceneFilePath.extension().string());
			L_CORE_WARN("Extension Expected: .lscene");
		}
		else
		{
			YAML::Node data;

			try {
				data = YAML::LoadFile(sceneFilePath.string());
			}
			catch (YAML::ParserException e) {
				L_CORE_ERROR("YAML-CPP Failed to Load Scene File: '{0}', {1}", sceneFilePath.string(), e.what());
				return false;
			}

			if (!data["Scene Name"]) {
				L_CORE_ERROR("Scene Name Node Not Correctly Declared in File: \'{0}\'", sceneFilePath.string());
				return false;
			}
			else {
				scene_ref->m_SceneConfig.Name = data["Scene Name"].as<std::string>();
			}

			if (!data["Scene Asset Directory"]) {
				L_CORE_ERROR("Scene Asset Directory Node Not Correctly Declared in File: \'{0}\'", sceneFilePath.string());
				return false;
			}
			else {
				scene_ref->m_SceneConfig.AssetDirectory = data["Scene Asset Directory"].as<std::string>();
			}

			if (!data["Scene Pipeline Type"]) {
				L_CORE_ERROR("Scene Pipeline Type Node Not Correctly Declared in File: \'{0}\'", sceneFilePath.string());
				return false;
			}
			else {

				scene_ref->m_SceneConfig.ScenePipelineType = (L_RENDER_PIPELINE)data["Scene Pipeline Type"].as<uint8_t>();

				switch (scene_ref->m_SceneConfig.ScenePipelineType) {
				case L_RENDER_PIPELINE::FORWARD:
					scene_ref->m_SceneConfig.ScenePipeline = std::make_shared<RenderPipeline>();
					break;
				case L_RENDER_PIPELINE::FORWARD_PLUS:
					scene_ref->m_SceneConfig.ScenePipeline = std::make_shared<ForwardPlusPipeline>();
					break;
				case L_RENDER_PIPELINE::DEFERRED:
					scene_ref->m_SceneConfig.ScenePipeline = std::make_shared<DeferredPipeline>();
					break;
				}
			}
			
			auto entities = data["Entities"];
			if (entities) {

				for (auto entity : entities) {
					
					// UUID
					UUID uuid = entity["Entity"].as<uint32_t>();
					std::string tag = entity["TagComponent"]["Tag"].as<std::string>();

					Entity deserializedEntity = scene_ref->CreateEntity(uuid, tag);

					L_CORE_INFO("Deserialising Entity: {0}", deserializedEntity.GetName());

					// Hierarchy
					auto hierarchy = entity["HierarchyComponent"];
					if (hierarchy) {
						auto& entityHierarchy = deserializedEntity.GetComponent<HierarchyComponent>();
						entityHierarchy.Deserialize(hierarchy);
					}

					// Script
					auto script = entity["ScriptComponent"];
					if (script) {
						auto& entityScript = deserializedEntity.AddComponent<ScriptComponent>();
						entityScript.Deserialize(script, uuid);
					}

					// Transform
					auto transform = entity["TransformComponent"];
					if (transform) {
						auto& entityTransform = deserializedEntity.GetComponent<TransformComponent>();
						entityTransform.Deserialize(transform);
					}

					// Camera
					auto camera = entity["CameraComponent"];
					if (camera) {

						auto& entityCamera = deserializedEntity.AddComponent<CameraComponent>();
						entityCamera.CameraInstance = std::make_shared<SceneCamera>();
						
						if (!entityCamera.Deserialize(camera))
							L_CORE_WARN("Deserialisation of Camera Component Not Complete.");
					}

					// Mesh Filter
					auto meshFilter = entity["MeshFilterComponent"];
					if (meshFilter) {

						auto& entityMeshFilter = deserializedEntity.AddComponent<AssetMeshFilter>();

						if (!entityMeshFilter.Deserialize(meshFilter))
							L_CORE_WARN("Deserialisation of Mesh Filter Not Complete.");
					}

					// Mesh Renderer
					auto meshRenderer = entity["MeshRendererComponent"];
					if (meshRenderer) {

						auto& entityMeshRenderer = deserializedEntity.AddComponent<AssetMeshRenderer>();

						if (!entityMeshRenderer.Deserialize(meshRenderer))
							L_CORE_WARN("Deserialisation of Mesh Renderer Not Complete.");
					}

					// Skybox Component and Skybox Material
					auto skybox = entity["SkyboxComponent"];
					if (skybox) {

						auto& skyboxComponent = deserializedEntity.AddComponent<SkyboxComponent>();

						if (!skyboxComponent.Deserialize(skybox))
							L_CORE_WARN("Deserialisation of Sky Box Not Complete.");
					}

					// Point Light
					auto pointLight = entity["PointLightComponent"];
					if (pointLight) {

						auto& entityPointLight = deserializedEntity.AddComponent<PointLightComponent>();

						if (!entityPointLight.Deserialize(pointLight))
							L_CORE_WARN("Deserialisation of Point Light Not Complete.");
					}

					// Spot Light
					auto spotLight = entity["SpotLightComponent"];
					if (spotLight) {

						auto& entitySpotLight = deserializedEntity.AddComponent<SpotLightComponent>();

						if (!entitySpotLight.Deserialize(spotLight))
							L_CORE_WARN("Deserialisation of Spot Light Not Complete.");
					}

					// Directional Light
					auto directionalLight = entity["DirectionalLightComponent"];
					if (directionalLight) {

						auto& entityDirectionalLight = deserializedEntity.AddComponent<DirectionalLightComponent>();

						if (!entityDirectionalLight.Deserialize(directionalLight))
							L_CORE_WARN("Deserialisation of Directional Light Not Complete.");
					}

					// Rigidbody
					auto rigidBody = entity["RigidbodyComponent"];
					if (rigidBody) {

						auto& entityRigidBody = deserializedEntity.AddComponent<RigidbodyComponent>();

						if (!entityRigidBody.Deserialize(rigidBody))
							L_CORE_WARN("Deserialisation of Rigidbody Not Complete.");
					}

					// Sphere Collider
					auto sphereCollider = entity["SphereColliderComponent"];
					if (sphereCollider) {

						auto& entitySphereCollider = deserializedEntity.AddComponent<SphereColliderComponent>();

						if (!entitySphereCollider.Deserialize(sphereCollider))
							L_CORE_WARN("Deserialisation of Sphere Collider Not Complete.");
					}

					// Box Collider
					auto boxCollider = entity["BoxColliderComponent"];
					if (boxCollider) {

						auto& entityBoxCollider = deserializedEntity.AddComponent<BoxColliderComponent>();

						if (!entityBoxCollider.Deserialize(boxCollider))
							L_CORE_WARN("Deserialisation of Box Collider Not Complete.");
					}
				}
			}
			//PhysicsSystem::Update(scene_ref);

			auto transform_update = scene_ref->GetAllEntitiesWith<TransformComponent>();
			for (const auto& entity_handle : transform_update) {
				transform_update.get<TransformComponent>(entity_handle).SetPosition(transform_update.get<TransformComponent>(entity_handle).GetLocalPosition());
			}

			// Update Camera Component
			Entity camera_entity = scene_ref->GetPrimaryCameraEntity();

			if (!camera_entity) {
				auto camera = scene_ref->CreateEntity("Main Camera");
				auto& camera_component = camera.AddComponent<CameraComponent>();
				camera_component.CameraInstance = std::make_shared<SceneCamera>();
			}

			// Calculate Overall Scene Octree
			OctreeBoundsConfig octree_config{};

			std::vector<OctreeBounds<Entity>::OctreeData> data_sources;

			auto bounds_view_mesh = scene_ref->GetAllEntitiesWith<AssetMeshFilter, AssetMeshRenderer>();
			for (const auto& entity_handle : bounds_view_mesh) {
				auto& mesh_filter = bounds_view_mesh.get<AssetMeshFilter>(entity_handle);

				// Ensure the AABB is up to date
				mesh_filter.UpdateTransformedAABB();

				const auto& aabb = mesh_filter.TransformedAABB;

				data_sources.push_back(std::make_shared<OctreeDataSource<Entity>>(mesh_filter.GetEntity(), aabb));
			}

			octree_config.Looseness = 1.25f;
			octree_config.PreferredDataSourceLimit = 8;

			// Create the octree and insert data sources
			scene_ref->m_Octree = std::make_shared<OctreeBounds<Entity>>(octree_config, data_sources);
			
			return true;
		}
		return false;
	}

}

#pragma warning( pop )