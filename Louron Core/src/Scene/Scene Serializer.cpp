#include "Scene Serializer.h"

// Louron Core Headers
#include "Scene.h"
#include "Entity.h"
#include "Resource Manager.h"

#include "Components/Components.h"
#include "Components/Light.h"
#include "Components/Skybox.h"
#include "Components/UUID.h"

#include "Components/Physics/Collider.h"
#include "Components/Physics/Rigidbody.h"
#include "Components/Physics/PhysicsWrappers.h"

#include "Scene Systems/Physics System.h"
#include "Scene Systems/Transform System.h"

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
		out << YAML::Key << "Entity" << YAML::Value << entity.GetUUID();

		if (entity.HasComponent<TagComponent>()) {

			out << YAML::Key << "TagComponent";
			out << YAML::BeginMap;

			const std::string& tag = entity.GetComponent<TagComponent>().Tag;
			out << YAML::Key << "Tag" << YAML::Value << tag;

			out << YAML::EndMap;
		}

		if (entity.HasComponent<Transform>()) {

			out << YAML::Key << "TransformComponent";
			out << YAML::BeginMap;

			Transform& transform = entity.GetComponent<Transform>();
			
			glm::vec3 v = transform.GetLocalPosition();
			out << YAML::Key << "Translation" << YAML::Value << YAML::Flow 
				<< YAML::BeginSeq 
					<< v.x 
					<< v.y 
					<< v.z 
				<< YAML::EndSeq;
			
			v = transform.GetLocalRotation();
			out << YAML::Key << "Rotation" << YAML::Value << YAML::Flow
				<< YAML::BeginSeq
				<< v.x
				<< v.y
				<< v.z
				<< YAML::EndSeq;
			
			v = transform.GetLocalScale();
			out << YAML::Key << "Scale" << YAML::Value << YAML::Flow
				<< YAML::BeginSeq
				<< v.x
				<< v.y
				<< v.z
				<< YAML::EndSeq;

			out << YAML::EndMap;
		}

		if (entity.HasComponent<CameraComponent>()) {

			out << YAML::Key << "CameraComponent";
			out << YAML::BeginMap;

			CameraComponent& cameraComponent = entity.GetComponent<CameraComponent>();
			{
				std::shared_ptr<Camera>& camera = cameraComponent.Camera;

				out << YAML::Key << "Camera" << YAML::Value;
				out << YAML::BeginMap;

				{
					out << YAML::Key << "FOV" << YAML::Value << camera->FOV;
					out << YAML::Key << "MovementToggle" << YAML::Value << camera->m_Movement;
					out << YAML::Key << "MovementSpeed" << YAML::Value << camera->MovementSpeed;
					out << YAML::Key << "MovementYDamp" << YAML::Value << camera->MovementYDamp;
					out << YAML::Key << "MouseSensitivity" << YAML::Value << camera->MouseSensitivity;
					out << YAML::Key << "MouseToggledOff" << YAML::Value << camera->MouseToggledOff;
					
					glm::vec3 v = camera->GetPosition();
					out << YAML::Key << "Position" << YAML::Value << YAML::Flow
						<< YAML::BeginSeq
						<< v.x
						<< v.y
						<< v.z
						<< YAML::EndSeq;

					out << YAML::Key << "Yaw" << YAML::Value << camera->GetYaw();
					out << YAML::Key << "Pitch" << YAML::Value << camera->GetPitch();
				}

				out << YAML::EndMap;
			}

			out << YAML::Key << "Primary" << YAML::Value << cameraComponent.Primary;
			out << YAML::Key << "ClearFlag" << YAML::Value << (int)cameraComponent.ClearFlags;

			out << YAML::EndMap;
		}

		if (entity.HasComponent<MeshRenderer>()) {

			out << YAML::Key << "MeshRendererComponent";
			out << YAML::BeginMap;

			const MeshRenderer& meshRenderer = entity.GetComponent<MeshRenderer>();
			out << YAML::Key << "MeshActive" << YAML::Value << meshRenderer.active;
			out << YAML::Key << "MeshFilePath" << YAML::Value << meshRenderer.GetPath();

			out << YAML::EndMap;
		}

		if (entity.HasComponent<SkyboxComponent>()) {

			const SkyboxComponent& skyboxComponent = entity.GetComponent<SkyboxComponent>();
			if (!skyboxComponent.Material->GetMaterialFilePath().empty()) {

				out << YAML::Key << "SkyboxComponent";
				out << YAML::BeginMap;

				out << YAML::Key << "MaterialFilePath" << YAML::Value << 
					skyboxComponent.Material->GetMaterialFilePath().string().substr(
						skyboxComponent.Material->GetMaterialFilePath().string().find(
							m_Scene.lock()->GetConfig().AssetDirectory.string()) + m_Scene.lock()->GetConfig().AssetDirectory.string().length()
					);

				{
					YAML::Emitter materialOut;
					materialOut << YAML::BeginMap;
					materialOut << YAML::Key << "Material Name" << YAML::Value << skyboxComponent.Material->GetName();
					materialOut << YAML::Key << "Material Type" << YAML::Value << "SkyboxMaterial";
					materialOut << YAML::Key << "TextureFilePaths" << YAML::Value << YAML::BeginMap;

					const std::array<std::filesystem::path, 6>& fsArray = skyboxComponent.Material->GetTextureFilePaths();

					std::unordered_map<int, std::string> indexKeyMap{
						{ 0, "Right"},
						{ 1, "Left"},
						{ 2, "Top"},
						{ 3, "Bottom"},
						{ 4, "Back"},
						{ 5, "Front"},
					};

					for (auto& pair : indexKeyMap) {
						materialOut << 
						YAML::Key	<< pair.second << 
						YAML::Value << fsArray[pair.first].string().substr(fsArray[pair.first].string().find(m_Scene.lock()->GetConfig().AssetDirectory.string()) + m_Scene.lock()->GetConfig().AssetDirectory.string().length());
					}

					materialOut << YAML::EndMap << YAML::EndMap;

					std::filesystem::create_directories(skyboxComponent.Material->GetMaterialFilePath().parent_path());

					std::ofstream fout(skyboxComponent.Material->GetMaterialFilePath());
					fout << materialOut.c_str();

					L_CORE_INFO("Skybox Material ({0}) Saved At: {1}", skyboxComponent.Material->GetMaterialFilePath().filename().replace_extension().string(), skyboxComponent.Material->GetMaterialFilePath().string());
				}

				out << YAML::EndMap;
			}
			else
				L_CORE_ERROR("Could Not Save SkyBox Information - No SkyBoxMaterial FilePath Specified!");
		}

		if (entity.HasComponent<PointLightComponent>()) {

			out << YAML::Key << "PointLightComponent";
			out << YAML::BeginMap;

			const PointLightComponent& light = entity.GetComponent<PointLightComponent>();

			glm::vec4 v = light.position;
			out << YAML::Key << "Position" << YAML::Value << YAML::Flow
				<< YAML::BeginSeq
				<< v.x
				<< v.y
				<< v.z
				<< v.w
				<< YAML::EndSeq;

			v = light.ambient;
			out << YAML::Key << "Ambient" << YAML::Value << YAML::Flow
				<< YAML::BeginSeq
				<< v.x
				<< v.y
				<< v.z
				<< v.w
				<< YAML::EndSeq;

			
			v = light.diffuse;
			out << YAML::Key << "Diffuse" << YAML::Value << YAML::Flow
				<< YAML::BeginSeq
				<< v.x
				<< v.y
				<< v.z
				<< v.w
				<< YAML::EndSeq;
			
			v = light.specular;
			out << YAML::Key << "Specular" << YAML::Value << YAML::Flow
				<< YAML::BeginSeq
				<< v.x
				<< v.y
				<< v.z
				<< v.w
				<< YAML::EndSeq;

			out << YAML::Key << "Radius" << YAML::Value << light.lightProperties.radius;
			out << YAML::Key << "Intensity" << YAML::Value << light.lightProperties.intensity;
			out << YAML::Key << "Active" << YAML::Value << light.lightProperties.active;

			out << YAML::EndMap;
		}
		
		if (entity.HasComponent<SpotLightComponent>()) {

			out << YAML::Key << "SpotLightComponent";
			out << YAML::BeginMap;

			const SpotLightComponent& light = entity.GetComponent<SpotLightComponent>();

			glm::vec4 v = light.position;
			out << YAML::Key << "Position" << YAML::Value << YAML::Flow
				<< YAML::BeginSeq
				<< v.x
				<< v.y
				<< v.z
				<< v.w
				<< YAML::EndSeq;
			
			v = light.direction;
			out << YAML::Key << "Direction" << YAML::Value << YAML::Flow
				<< YAML::BeginSeq
				<< v.x
				<< v.y
				<< v.z
				<< v.w
				<< YAML::EndSeq;

			v = light.ambient;
			out << YAML::Key << "Ambient" << YAML::Value << YAML::Flow
				<< YAML::BeginSeq
				<< v.x
				<< v.y
				<< v.z
				<< v.w
				<< YAML::EndSeq;

			v = light.diffuse;
			out << YAML::Key << "Diffuse" << YAML::Value << YAML::Flow
				<< YAML::BeginSeq
				<< v.x
				<< v.y
				<< v.z
				<< v.w
				<< YAML::EndSeq;

			v = light.specular;
			out << YAML::Key << "Specular" << YAML::Value << YAML::Flow
				<< YAML::BeginSeq
				<< v.x
				<< v.y
				<< v.z
				<< v.w
				<< YAML::EndSeq;

			out << YAML::Key << "Range" << YAML::Value << light.lightProperties.range;
			out << YAML::Key << "Angle" << YAML::Value << light.lightProperties.angle;
			out << YAML::Key << "Intensity" << YAML::Value << light.lightProperties.intensity;
			out << YAML::Key << "Active" << YAML::Value << light.lightProperties.active;

			out << YAML::EndMap;
		}
		
		if (entity.HasComponent<DirectionalLightComponent>()) {

			out << YAML::Key << "DirectionalLightComponent";
			out << YAML::BeginMap;

			const DirectionalLightComponent& light = entity.GetComponent<DirectionalLightComponent>();

			glm::vec4 v = light.ambient;
			out << YAML::Key << "Ambient" << YAML::Value << YAML::Flow
				<< YAML::BeginSeq
				<< v.x
				<< v.y
				<< v.z
				<< v.w
				<< YAML::EndSeq;
			
			v = light.diffuse;
			out << YAML::Key << "Diffuse" << YAML::Value << YAML::Flow
				<< YAML::BeginSeq
				<< v.x
				<< v.y
				<< v.z
				<< v.w
				<< YAML::EndSeq;

			v = light.specular;
			out << YAML::Key << "Specular" << YAML::Value << YAML::Flow
				<< YAML::BeginSeq
				<< v.x
				<< v.y
				<< v.z
				<< v.w
				<< YAML::EndSeq;
			
			out << YAML::EndMap;
		}

		if (entity.HasComponent<Rigidbody>()) {

			out << YAML::Key << "RigidbodyComponent";
			out << YAML::BeginMap;

			const Rigidbody& rigidBody = entity.GetComponent<Rigidbody>();

			out << YAML::Key << "Mass" << YAML::Value << rigidBody.GetMass();
			out << YAML::Key << "Drag" << YAML::Value << rigidBody.GetDrag();
			out << YAML::Key << "AngularDrag" << YAML::Value << rigidBody.GetAngularDrag();

			out << YAML::Key << "AutomaticCentreOfMass" << YAML::Value << rigidBody.IsAutomaticCentreOfMassEnabled();
			out << YAML::Key << "UseGravity" << YAML::Value << rigidBody.IsGravityEnabled();
			out << YAML::Key << "IsKinematic" << YAML::Value << rigidBody.IsKinematicEnabled();

			glm::vec3 v = rigidBody.GetPositionConstraint();
			out << YAML::Key << "PositionConstraint" << YAML::Value << YAML::Flow
				<< YAML::BeginSeq
				<< v.x
				<< v.y
				<< v.z
				<< YAML::EndSeq;

			v = rigidBody.GetRotationConstraint();
			out << YAML::Key << "RotationConstraint" << YAML::Value << YAML::Flow
				<< YAML::BeginSeq
				<< v.x
				<< v.y
				<< v.z
				<< YAML::EndSeq;

			out << YAML::EndMap;
		}

		if (entity.HasComponent<SphereCollider>()) {

			out << YAML::Key << "SphereColliderComponent";
			out << YAML::BeginMap;

			const SphereCollider& sphereCollider = entity.GetComponent<SphereCollider>();

			out << YAML::Key << "IsTrigger" << YAML::Value << sphereCollider.GetIsTrigger();

			out << YAML::Key << "Radius" << YAML::Value << sphereCollider.GetRadius();

			glm::vec3 v = sphereCollider.GetCentre();
			out << YAML::Key << "Centre" << YAML::Value << YAML::Flow
				<< YAML::BeginSeq
				<< v.x
				<< v.y
				<< v.z
				<< YAML::EndSeq;

			std::shared_ptr<PhysicsMaterial> tempMaterial = sphereCollider.GetMaterial();
			out << YAML::Key << "MaterialProperties" << YAML::Value << YAML::Flow
				<< YAML::BeginSeq
				<< tempMaterial->GetStaticFriction()
				<< tempMaterial->GetDynamicFriction()
				<< tempMaterial->GetBounciness()
				<< YAML::EndSeq;

			out << YAML::EndMap;
		}

		if (entity.HasComponent<BoxCollider>()) {

			out << YAML::Key << "BoxColliderComponent";
			out << YAML::BeginMap;

			const BoxCollider& boxCollider = entity.GetComponent<BoxCollider>();

			out << YAML::Key << "IsTrigger" << YAML::Value << boxCollider.GetIsTrigger();

			glm::vec3 v = boxCollider.GetCentre();
			out << YAML::Key << "Centre" << YAML::Value << YAML::Flow
				<< YAML::BeginSeq
				<< v.x
				<< v.y
				<< v.z
				<< YAML::EndSeq;

			v = boxCollider.GetSize();
			out << YAML::Key << "Size" << YAML::Value << YAML::Flow
				<< YAML::BeginSeq
				<< v.x
				<< v.y
				<< v.z
				<< YAML::EndSeq;

			std::shared_ptr<PhysicsMaterial> tempMaterial = boxCollider.GetMaterial();
			out << YAML::Key << "MaterialProperties" << YAML::Value << YAML::Flow
				<< YAML::BeginSeq
				<< tempMaterial->GetStaticFriction()
				<< tempMaterial->GetDynamicFriction()
				<< tempMaterial->GetBounciness()
				<< YAML::EndSeq;

			out << YAML::EndMap;
		}

		out << YAML::EndMap;
	}

	bool SceneSerializer::Serialize(const std::filesystem::path& sceneFilePath) {

		auto scene_ref = m_Scene.lock();
		if (!scene_ref) {
			L_CORE_ERROR("Scene Invalid - Cannot Serialize.");
			return false;
		}

		std::filesystem::path outFilePath = (sceneFilePath.empty()) ? scene_ref->m_SceneFilePath : sceneFilePath;

		if (outFilePath.extension() != ".lscene") {

			L_CORE_WARN("Incompatible Scene File Extension");
			L_CORE_WARN("Extension Used: {0}", outFilePath.extension().string());
			L_CORE_WARN("Extension Expected: .lscene");
		}
		else
		{

			YAML::Emitter out;
			out << YAML::BeginMap;
			out << YAML::Key << "Scene Name" << YAML::Value << scene_ref->m_SceneConfig.Name;
			out << YAML::Key << "Scene Asset Directory" << YAML::Value << scene_ref->m_SceneConfig.AssetDirectory.string();
			out << YAML::Key << "Scene Pipeline Type" << YAML::Value << (int)scene_ref->m_SceneConfig.ScenePipelineType;

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
		return false;
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
				scene_ref->m_SceneConfig.ScenePipelineType = (L_RENDER_PIPELINE)data["Scene Pipeline Type"].as<int>();

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
					UUID uuid = entity["Entity"].as<UUID>();

					// Tag
					std::string name;
					auto tag = entity["TagComponent"];
					if (tag) name = tag["Tag"].as<std::string>();

					Entity deserializedEntity = scene_ref->CreateEntity(uuid, name);

					// Transform
					auto transform = entity["TransformComponent"];
					if (transform) {
						auto& entityTransform = deserializedEntity.GetComponent<Transform>();

						entityTransform.SetPosition(transform["Translation"].as<glm::vec3>());
						entityTransform.SetRotation(transform["Rotation"].as<glm::vec3>());
						entityTransform.SetScale(transform["Scale"].as<glm::vec3>());
					}

					// Camera
					auto camera = entity["CameraComponent"];
					if (camera) {

						auto& entityCamera = deserializedEntity.AddComponent<CameraComponent>();
						
						entityCamera.Camera = std::make_shared<Louron::Camera>(camera["Camera"]["Position"].as<glm::vec3>());
						entityCamera.Camera->FOV = camera["Camera"]["FOV"].as<float>();
						entityCamera.Camera->m_Movement = camera["Camera"]["MovementToggle"].as<bool>();
						entityCamera.Camera->MovementSpeed = camera["Camera"]["MovementSpeed"].as<float>();
						entityCamera.Camera->MovementYDamp = camera["Camera"]["MovementYDamp"].as<float>();
						entityCamera.Camera->MouseSensitivity = camera["Camera"]["MouseSensitivity"].as<float>();
						entityCamera.Camera->MouseToggledOff = camera["Camera"]["MouseToggledOff"].as<bool>();

						entityCamera.Camera->SetYaw(camera["Camera"]["Yaw"].as<float>());
						entityCamera.Camera->SetPitch(camera["Camera"]["Pitch"].as<float>());

						entityCamera.Primary = camera["Primary"].as<bool>();
						entityCamera.ClearFlags = (CameraClearFlags)camera["ClearFlag"].as<int>();

					}

					// Mesh Renderer and Mesh Filter
					auto meshRenderer = entity["MeshRendererComponent"];
					if (meshRenderer) {

						scene_ref->m_SceneConfig.SceneResourceManager->LoadMesh(meshRenderer["MeshFilePath"].as<std::string>().c_str(), Louron::Engine::Get().GetShaderLibrary().GetShader("FP_Material_BP_Shader"));

						deserializedEntity.AddComponent<MeshFilter>().LinkMeshFilter(scene_ref->m_SceneConfig.SceneResourceManager->GetMeshFilter(meshRenderer["MeshFilePath"].as<std::string>()));
						auto& entityMeshRenderer = deserializedEntity.AddComponent<MeshRenderer>();
						entityMeshRenderer.LinkMeshRenderer(scene_ref->m_SceneConfig.SceneResourceManager->GetMeshRenderer(meshRenderer["MeshFilePath"].as<std::string>()));
						entityMeshRenderer.active = meshRenderer["MeshActive"].as<bool>();
					}

					// Skybox Component and Skybox Material
					auto skybox = entity["SkyboxComponent"];
					if (skybox) {

						SkyboxComponent& skyboxComponent = deserializedEntity.AddComponent<SkyboxComponent>();
						skyboxComponent.Material = DeserializeSkyboxMaterial(scene_ref->m_SceneConfig.AssetDirectory / skybox["MaterialFilePath"].as<std::string>());
					}

					// Point Light

					auto pointLight = entity["PointLightComponent"];
					if (pointLight) {

						auto& entityPointLight = deserializedEntity.AddComponent<PointLightComponent>();
						entityPointLight.position = pointLight["Position"].as<glm::vec4>();
						entityPointLight.ambient = pointLight["Ambient"].as<glm::vec4>();
						entityPointLight.diffuse = pointLight["Diffuse"].as<glm::vec4>();
						entityPointLight.specular = pointLight["Specular"].as<glm::vec4>();

						entityPointLight.lightProperties.radius = pointLight["Radius"].as<GLfloat>();
						entityPointLight.lightProperties.intensity = pointLight["Intensity"].as<GLfloat>();
						entityPointLight.lightProperties.active = pointLight["Active"].as<GLint>();
					}

					// Spot Light

					auto spotLight = entity["SpotLightComponent"];
					if (spotLight) {

						auto& entitySpotLight = deserializedEntity.AddComponent<SpotLightComponent>();
						entitySpotLight.position = spotLight["Position"].as<glm::vec4>();
						entitySpotLight.direction = spotLight["Direction"].as<glm::vec4>();
						entitySpotLight.ambient = spotLight["Ambient"].as<glm::vec4>();
						entitySpotLight.diffuse = spotLight["Diffuse"].as<glm::vec4>();
						entitySpotLight.specular = spotLight["Specular"].as<glm::vec4>();

						entitySpotLight.lightProperties.range = spotLight["Range"].as<GLfloat>();
						entitySpotLight.lightProperties.angle = spotLight["Angle"].as<GLfloat>();
						entitySpotLight.lightProperties.intensity = spotLight["Intensity"].as<GLfloat>();
						entitySpotLight.lightProperties.active = spotLight["Active"].as<GLint>();
					}

					// Directional Light

					auto directionalLight = entity["DirectionalLightComponent"];
					if (directionalLight) {

						auto& entityDirectionalLight = deserializedEntity.AddComponent<DirectionalLightComponent>();
						entityDirectionalLight.ambient = directionalLight["Ambient"].as<glm::vec4>();
						entityDirectionalLight.diffuse = directionalLight["Diffuse"].as<glm::vec4>();
						entityDirectionalLight.specular = directionalLight["Specular"].as<glm::vec4>();
					}

					auto rigidBody = entity["RigidbodyComponent"];
					if (rigidBody) {

						auto& entityRigidBody = deserializedEntity.AddComponent<Rigidbody>();

						entityRigidBody.SetMass(rigidBody["Mass"].as<float>());
						entityRigidBody.SetDrag(rigidBody["Drag"].as<float>());
						entityRigidBody.SetAngularDrag(rigidBody["AngularDrag"].as<float>());

						entityRigidBody.SetAutomaticCentreOfMass(rigidBody["AutomaticCentreOfMass"].as<bool>());
						entityRigidBody.SetGravity(rigidBody["UseGravity"].as<bool>());
						entityRigidBody.SetKinematic(rigidBody["IsKinematic"].as<bool>());

						entityRigidBody.SetPositionConstraint(rigidBody["PositionConstraint"].as<glm::vec3>());
						entityRigidBody.SetRotationConstraint(rigidBody["RotationConstraint"].as<glm::vec3>());

						entityRigidBody.GetActor()->AddFlag(RigidbodyFlag_TransformUpdated);
						entityRigidBody.GetActor()->AddFlag(RigidbodyFlag_ShapesUpdated);
					}

					auto sphereCollider = entity["SphereColliderComponent"];
					if (sphereCollider) {

						auto& entitySphereCollider = deserializedEntity.AddComponent<SphereCollider>();
						entitySphereCollider.SetIsTrigger(sphereCollider["IsTrigger"].as<bool>());

						entitySphereCollider.SetRadius(sphereCollider["Radius"].as<float>());

						entitySphereCollider.SetCentre(sphereCollider["Centre"].as<glm::vec3>());

						glm::vec3 materialProps = sphereCollider["MaterialProperties"].as<glm::vec3>();
						entitySphereCollider.GetMaterial()->SetStaticFriction(materialProps.x);
						entitySphereCollider.GetMaterial()->SetDynamicFriction(materialProps.y);
						entitySphereCollider.GetMaterial()->SetBounciness(materialProps.z);

						entitySphereCollider.GetShape()->AddFlag(ColliderFlag_RigidbodyUpdated);
						entitySphereCollider.GetShape()->AddFlag(ColliderFlag_ShapePropsUpdated);
						entitySphereCollider.GetShape()->AddFlag(ColliderFlag_TransformUpdated);
					}

					auto boxCollider = entity["BoxColliderComponent"];
					if (boxCollider) {

						auto& entityBoxCollider = deserializedEntity.AddComponent<BoxCollider>();
						entityBoxCollider.SetIsTrigger(boxCollider["IsTrigger"].as<bool>());

						entityBoxCollider.SetSize(boxCollider["Size"].as<glm::vec3>());

						entityBoxCollider.SetCentre(boxCollider["Centre"].as<glm::vec3>());

						glm::vec3 materialProps = boxCollider["MaterialProperties"].as<glm::vec3>();
						entityBoxCollider.GetMaterial()->SetStaticFriction(materialProps.x);
						entityBoxCollider.GetMaterial()->SetDynamicFriction(materialProps.y);
						entityBoxCollider.GetMaterial()->SetBounciness(materialProps.z);

						entityBoxCollider.GetShape()->AddFlag(ColliderFlag_RigidbodyUpdated);
						entityBoxCollider.GetShape()->AddFlag(ColliderFlag_ShapePropsUpdated);
						entityBoxCollider.GetShape()->AddFlag(ColliderFlag_TransformUpdated);
					}
				}
			}
			TransformSystem::Update(scene_ref);
			PhysicsSystem::Update(scene_ref);
			return true;
		}
		return false;
	}

	std::shared_ptr<SkyboxMaterial> SceneSerializer::DeserializeSkyboxMaterial(const std::filesystem::path& filePath) {
		
		if (filePath.extension() != ".lmaterial") {

			L_CORE_WARN("Incompatible Material File Extension");
			L_CORE_WARN("Extension Used: {0}", filePath.extension().string());
			L_CORE_WARN("Extension Expected: .lmaterial");
		}
		else
		{
			YAML::Node data;

			try {
				data = YAML::LoadFile(filePath.string());
			}
			catch (YAML::ParserException e) {
				L_CORE_ERROR("YAML-CPP Failed to Load Scene File: '{0}', {1}", filePath.string(), e.what());
				return nullptr;
			}

			if (!data["Material Type"] || data["Material Type"].as<std::string>() != "SkyboxMaterial") {
				L_CORE_ERROR("Material Type Node is Not Skybox Material: '{0}'", filePath.string());
				return nullptr;
			}

			if (!data["Material Name"]) {
				L_CORE_ERROR("Material Name Node Not Correctly Declared in File: '{0}'", filePath.string());
				return nullptr;
			}

			if (!data["TextureFilePaths"]) {
				L_CORE_ERROR("TextureFilePaths Node Not Correctly Declared in File: '{0}'", filePath.string());
				return nullptr;
			}
			else {

				std::shared_ptr<SkyboxMaterial> material = std::make_shared<SkyboxMaterial>();
				std::array<std::filesystem::path, 6> textureFilePathArray;

				std::unordered_map<int, std::string> indexKeyMap{
				
					{ 0, "Right"},
					{ 1, "Left"},
					{ 2, "Top"},
					{ 3, "Bottom"},
					{ 4, "Back"},
					{ 5, "Front"},
				
				};

				auto texturesData = data["TextureFilePaths"];
				if (texturesData)
					for (auto& pair : indexKeyMap) 
						if(texturesData[pair.second])
							textureFilePathArray[pair.first] = m_Scene.lock()->m_SceneConfig.AssetDirectory / texturesData[pair.second].as<std::string>();
				
				material->LoadSkybox(textureFilePathArray);
				material->SetName(data["Material Name"].as<std::string>());
				material->SetMaterialFilePath(filePath);

				return material;
			}
		}
		return nullptr;
	}
}

#pragma warning( pop )