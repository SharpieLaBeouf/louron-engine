#include "Scene Serializer.h"

// Louron Core Headers
#include "Entity.h"

#include "Components/UUID.h"
#include "Components/Components.h"
#include "Components/Skybox.h"

#include "../Renderer/RendererPipeline.h"

// C++ Standard Library Headers
#include <fstream>

// External Vendor Library Headers
#include <glm/gtx/string_cast.hpp>

#pragma warning( push )
#pragma warning( disable : 4099)

#define YAML_CPP_STATIC_DEFINE
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
	
	SceneSerializer::SceneSerializer(const std::shared_ptr<Scene>& scene) : m_Scene(scene) { }
	
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

			out << YAML::Key << "SkyboxComponent";
			out << YAML::BeginMap;

			const SkyboxComponent& skyboxComponent = entity.GetComponent<SkyboxComponent>();
			out << YAML::Key << "MaterialFilePath" << YAML::Value << skyboxComponent.Material->GetMaterialFilePath().string().substr(skyboxComponent.Material->GetMaterialFilePath().string().find(m_Scene->GetConfig().AssetDirectory.string()) + m_Scene->GetConfig().AssetDirectory.string().length());

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

				for (auto& pair : indexKeyMap)
					materialOut << 
					YAML::Key	<< pair.second << 
					YAML::Value << fsArray[pair.first].string().substr(fsArray[pair.first].string().find(m_Scene->GetConfig().AssetDirectory.string()) + m_Scene->GetConfig().AssetDirectory.string().length());

				materialOut << YAML::EndMap << YAML::EndMap;

				std::filesystem::create_directories(skyboxComponent.Material->GetMaterialFilePath().parent_path());

				std::ofstream fout(skyboxComponent.Material->GetMaterialFilePath());
				fout << materialOut.c_str();

				L_CORE_INFO("Skybox Material ({0}) Saved At: {1}", skyboxComponent.Material->GetMaterialFilePath().filename().replace_extension().string(), skyboxComponent.Material->GetMaterialFilePath().string());
			}

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

	void SceneSerializer::Serialize(const std::filesystem::path& sceneFilePath) {

		std::filesystem::path outFilePath = (sceneFilePath.empty()) ? m_Scene->m_SceneFilePath : sceneFilePath;

		L_CORE_ASSERT((outFilePath.extension() == ".lscene"), "Incompatible Scene File Extension! Extension used: " + outFilePath.extension().string() + ", Extension Required: .lscene");

		if (outFilePath.extension() == ".lscene") {

			YAML::Emitter out;
			out << YAML::BeginMap;
			out << YAML::Key << "Scene Name" << YAML::Value << m_Scene->m_SceneConfig.Name;
			out << YAML::Key << "Scene Asset Directory" << YAML::Value << m_Scene->m_SceneConfig.AssetDirectory.string();
			out << YAML::Key << "Scene Pipeline Type" << YAML::Value << (int)m_Scene->m_SceneConfig.ScenePipelineType;

			out << YAML::Key << "Entities" << YAML::Value << YAML::BeginSeq;

			m_Scene->m_Registry.each([&](auto entityID)
				{

					Entity entity = { entityID, m_Scene.get() };
					if (!entity)
						return;

					SerializeEntity(out, entity);
				});

			out << YAML::EndSeq << YAML::EndMap;

			std::filesystem::create_directories(outFilePath.parent_path());

			std::ofstream fout(outFilePath);
			fout << out.c_str();
		}
	}

	bool SceneSerializer::Deserialize(const std::filesystem::path& sceneFilePath) {

		L_CORE_ASSERT((sceneFilePath.extension() == ".lscene"), "Incompatible Scene File Extension! Extension used: " + sceneFilePath.extension().string() + ", Extension Required: .lscene");

		if (sceneFilePath.extension() == ".lscene") {

			YAML::Node data;

			try {
				data = YAML::LoadFile(sceneFilePath.string());
			}
			catch (YAML::ParserException e) {
				L_CORE_ASSERT(false, "YAML-CPP Failed to Load Scene File: " + sceneFilePath.string() + ", " + e.what());
				return false;
			}

			if (!data["Scene Name"]) {
				L_CORE_ASSERT(false, "Scene Name Node Not Correctly Declared in File : " + sceneFilePath.string());
				return false;
			}
			else {
				m_Scene->m_SceneConfig.Name = data["Scene Name"].as<std::string>();
			}

			if (!data["Scene Asset Directory"]) {
				L_CORE_ASSERT(false, "Scene Asset Directory Node Not Correctly Declared in File : " + sceneFilePath.string());
				return false;
			}
			else {
				m_Scene->m_SceneConfig.AssetDirectory = data["Scene Asset Directory"].as<std::string>();
			}

			if (!data["Scene Pipeline Type"]) {
				L_CORE_ASSERT(false, "Scene Pipeline Type Node Not Correctly Declared in File : " + sceneFilePath.string());
				return false;
			}
			else {
				m_Scene->m_SceneConfig.ScenePipelineType = (L_RENDER_PIPELINE)data["Scene Pipeline Type"].as<int>();

				switch (m_Scene->m_SceneConfig.ScenePipelineType) {
				case L_RENDER_PIPELINE::FORWARD:
					m_Scene->m_SceneConfig.ScenePipeline = std::make_shared<RenderPipeline>();
					break;
				case L_RENDER_PIPELINE::FORWARD_PLUS:
					m_Scene->m_SceneConfig.ScenePipeline = std::make_shared<ForwardPlusPipeline>();
					break;
				case L_RENDER_PIPELINE::DEFERRED:
					m_Scene->m_SceneConfig.ScenePipeline = std::make_shared<DeferredPipeline>();
					break;
				}
			}
			
			auto entities = data["Entities"];
			if (entities) {

				for (auto entity : entities) {

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

						entityCamera.Camera->SetYaw(camera["Camera"]["Yaw"].as<float>());
						entityCamera.Camera->SetPitch(camera["Camera"]["Pitch"].as<float>());

						entityCamera.Primary = camera["Primary"].as<bool>();
						entityCamera.ClearFlags = (L_CAMERA_CLEAR_FLAGS)camera["ClearFlag"].as<int>();

					}

					// Mesh Renderer and Mesh Filter
					auto meshRenderer = entity["MeshRendererComponent"];
					if (meshRenderer) {

						m_Scene->m_SceneConfig.SceneResourceManager->LoadMesh(meshRenderer["MeshFilePath"].as<std::string>().c_str(), Louron::Engine::Get().GetShaderLibrary().GetShader("FP_Material_BP_Shader"));

						deserializedEntity.AddComponent<MeshFilter>().LinkMeshFilter(m_Scene->m_SceneConfig.SceneResourceManager->GetMeshFilter(meshRenderer["MeshFilePath"].as<std::string>()));
						auto& entityMeshRenderer = deserializedEntity.AddComponent<MeshRenderer>();
						entityMeshRenderer.LinkMeshRenderer(m_Scene->m_SceneConfig.SceneResourceManager->GetMeshRenderer(meshRenderer["MeshFilePath"].as<std::string>()));
						entityMeshRenderer.active = meshRenderer["MeshActive"].as<bool>();
					}

					// Skybox Component and Skybox Material
					auto skybox = entity["SkyboxComponent"];
					if (skybox) {

						SkyboxComponent& skyboxComponent = deserializedEntity.AddComponent<SkyboxComponent>();
						skyboxComponent.Material = DeserializeSkyboxMaterial(m_Scene->m_SceneConfig.AssetDirectory / skybox["MaterialFilePath"].as<std::string>());
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
			return true;
		}

		return false;
	}

	std::shared_ptr<SkyboxMaterial> SceneSerializer::DeserializeSkyboxMaterial(const std::filesystem::path& filePath) {
		
		L_CORE_ASSERT((filePath.extension() == ".lmaterial"), "Incompatible Skybox Material File Extension! Extension used: " + filePath.extension().string() + ", Extension Required: .lmaterial");

		std::shared_ptr<SkyboxMaterial> material = std::make_shared<SkyboxMaterial>();
		if (filePath.extension() == ".lmaterial") {

			YAML::Node data;

			try {
				data = YAML::LoadFile(filePath.string());
			}
			catch (YAML::ParserException e) {
				L_CORE_ASSERT(false, "YAML-CPP Failed to Load Scene File: " + filePath.string() + ", " + e.what());
				return material;
			}

			if (!data["Material Type"] || data["Material Type"].as<std::string>() != "SkyboxMaterial") {
				L_CORE_ASSERT(false, "Material Type Node is Not Skybox Material: " + filePath.string());
				return material;
			}

			if (!data["Material Name"]) {
				L_CORE_ASSERT(false, "Material Name Node Not Correctly Declared in File: " + filePath.string());
				return material;
			}

			if (!data["TextureFilePaths"]) {
				L_CORE_ASSERT(false, "TextureFilePaths Node Not Correctly Declared in File: " + filePath.string());
				return material;
			}
			else {

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
							textureFilePathArray[pair.first] = m_Scene->m_SceneConfig.AssetDirectory / texturesData[pair.second].as<std::string>();
				
				material->LoadSkybox(textureFilePathArray);
				material->SetName(data["Material Name"].as<std::string>());
				material->SetMaterialFilePath(filePath);
			}
		}

		return material;
	}
}

#pragma warning( pop )