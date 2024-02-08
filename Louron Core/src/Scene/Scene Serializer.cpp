#include "Scene Serializer.h"

#include "UUID.h"
#include "Entity.h"
#include "Components.h"

#include <fstream>

#define YAML_CPP_STATIC_DEFINE
#include <yaml-cpp/yaml.h>

#include <glm/gtx/string_cast.hpp>

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
			node.push_back((uint64_t)uuid);
			return node;
		}

		static bool decode(const Node& node, Louron::UUID& uuid)
		{
			uuid = node.as<uint64_t>();
			return true;
		}
	};
}

namespace Louron {
	
	SceneSerializer::SceneSerializer(const std::shared_ptr<Scene>& scene) : m_Scene(scene)
	{

	}
	
	static void SerializeEntity(YAML::Emitter& out, Entity entity) {

		// TODO: implement UUID
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
			
			glm::vec3 v = transform.GetPosition();
			out << YAML::Key << "Translation" << YAML::Value << YAML::Flow 
				<< YAML::BeginSeq 
					<< v.x 
					<< v.y 
					<< v.z 
				<< YAML::EndSeq;
			
			v = transform.GetRotation();
			out << YAML::Key << "Rotation" << YAML::Value << YAML::Flow
				<< YAML::BeginSeq
				<< v.x
				<< v.y
				<< v.z
				<< YAML::EndSeq;
			
			v = transform.GetScale();
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

		out << YAML::EndMap;
	}

	void SceneSerializer::Serialize(const std::string& sceneFilePath) {

		std::string outFilePath;

		if (sceneFilePath == "")
			outFilePath = m_Scene->m_SceneConfig->AssetDirectory.string() + "Scenes/" + m_Scene->m_SceneConfig->Name + ".lscene";
		else
			outFilePath = sceneFilePath;

		auto extensionPos = outFilePath.rfind('.');
		std::string sceneExtension = (extensionPos != std::string::npos)
			? outFilePath.substr(extensionPos) : "No Extension Found";

		if (sceneExtension == ".lscene") {

			YAML::Emitter out;
			out << YAML::BeginMap;
			out << YAML::Key << "Scene Name" << YAML::Value << m_Scene->m_SceneConfig->Name;

			out << YAML::Key << "Entities" << YAML::Value << YAML::BeginSeq;

			m_Scene->m_Registry.each([&](auto entityID)
				{

					Entity entity = { entityID, m_Scene.get() };
					if (!entity)
						return;

					SerializeEntity(out, entity);

					m_Scene->DestroyEntity(entity);
				});

			out << YAML::EndMap;

			std::ofstream fout(outFilePath);
			fout << out.c_str();
		}
		else {
			L_CORE_ASSERT(false, "Incompatible Scene File Extension! Extension used: " + sceneExtension + ", Extension Required: .lscene");
		}

	}

	bool SceneSerializer::Deserialize(const std::string& sceneFilePath) {
		std::string outFilePath;

		if (sceneFilePath == "")
			outFilePath = m_Scene->m_SceneConfig->AssetDirectory.string() + "Scenes/" + m_Scene->m_SceneConfig->Name + ".lscene";
		else
			outFilePath = sceneFilePath;

		auto extensionPos = outFilePath.rfind('.');
		std::string sceneExtension = (extensionPos != std::string::npos)
			? outFilePath.substr(extensionPos) : "No Extension Found";

		if (sceneExtension == ".lscene") {

			YAML::Node data;

			try {
				data = YAML::LoadFile(outFilePath);
			}
			catch (YAML::ParserException e) {
				L_CORE_ASSERT(false, "Failed to load scene file: " + outFilePath);
			}

			if (!data["Scene Name"])
				return false;

			std::string sceneName = data["Scene Name"].as<std::string>();
			
			auto entities = data["Entities"];
			if (entities) {

				for (auto entity : entities){

					uint64_t uuid = entity["Entity"].as<uint64_t>();

					// Tag
					std::string name;
					auto tag = entity["TagComponent"];
					if (tag) 
						name = tag["Tag"].as<std::string>();

					Entity deserializedEntity = m_Scene->CreateEntity(uuid, name);

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
						entityCamera.Camera->MovementSpeed = camera["Camera"]["MovementSpeed"].as<float>();
						entityCamera.Camera->MovementYDamp = camera["Camera"]["MovementYDamp"].as<float>();
						entityCamera.Camera->MouseSensitivity = camera["Camera"]["MouseSensitivity"].as<float>();
						entityCamera.Camera->MouseToggledOff = camera["Camera"]["MouseToggledOff"].as<bool>();

						entityCamera.Camera->setYaw(camera["Camera"]["Yaw"].as<float>());
						entityCamera.Camera->setPitch(camera["Camera"]["Pitch"].as<float>());

						entityCamera.Primary = camera["Primary"].as<bool>();

					}

					// Mesh Renderer and Mesh Filter
					auto meshRenderer = entity["MeshRendererComponent"];
					if (meshRenderer) {

						m_Scene->m_SceneConfig->ResourceManager->LoadMesh(meshRenderer["MeshFilePath"].as<std::string>().c_str(), Louron::Engine::Get().GetShaderLibrary().GetShader("FP_Material_BP_Shader"));

						deserializedEntity.AddComponent<MeshFilter>().LinkMeshFilter(m_Scene->m_SceneConfig->ResourceManager->GetMeshFilter(meshRenderer["MeshFilePath"].as<std::string>()));
						auto& entityMeshRenderer = deserializedEntity.AddComponent<MeshRenderer>();
						entityMeshRenderer.LinkMeshRenderer(m_Scene->m_SceneConfig->ResourceManager->GetMeshRenderer(meshRenderer["MeshFilePath"].as<std::string>()));
						entityMeshRenderer.active = meshRenderer["MeshActive"].as<bool>();
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

				}
			}
		}
		else {
			L_CORE_ASSERT(false, "Incompatible Scene File Extension! Extension used: " + sceneExtension + ", Extension Required: .lscene");
		}

		return true;
	}
}

