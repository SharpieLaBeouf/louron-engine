#include "Skybox.h"

// Louron Core Headers
#include "Camera.h"

#include "../../Debug/Assert.h"

#include "../../Project/Project.h"

// C++ Standard Library Headers

// External Vendor Library Headers
#define STB_IMAGE_STATIC
#define STB_IMAGE_IMPLEMENTATION    
#include <stb_image/stb_image.h>

#ifndef YAML_CPP_STATIC_DEFINE
#define YAML_CPP_STATIC_DEFINE
#endif
#include <yaml-cpp/yaml.h>

namespace Louron {
	
#pragma region SkyboxMaterial

	/// <summary>
	/// Initialise the Skybox Material in OpenGL.
	/// </summary>
	SkyboxMaterial::SkyboxMaterial() {
		glGenTextures(1, &m_SkyboxID);
		glBindTexture(GL_TEXTURE_CUBE_MAP, m_SkyboxID);

		m_MaterialShader->Bind();
		m_MaterialShader->SetInt("u_Skybox", 0);
	}

	/// <summary>
	/// Load all textures and setup the skybox.
	/// </summary>
	/// <param name="textures">A vector list of filepaths. Please ensure 
	/// filepaths are ordered as such: right, left, top, bottom, back, front.</param>
	GLboolean SkyboxMaterial::LoadSkybox(const std::array<std::filesystem::path, 6>& textures) {
		
		m_TextureFilePaths = textures;

		glBindTexture(GL_TEXTURE_CUBE_MAP, m_SkyboxID);

		int width, height, nrChannels;
		for (unsigned int i = 0; i < m_TextureFilePaths.size(); i++)
		{
			unsigned char* data = stbi_load(m_TextureFilePaths[i].string().c_str(), &width, &height, &nrChannels, 0);
			if (data)
			{
				GLenum format = GL_RGBA;

				switch (nrChannels) {
				case 1:
					format = GL_RED;
					break;
				case 3:
					format = GL_RGB;
					break;
				case 4:
					format = GL_RGBA;
					break;
				}
				glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
				stbi_image_free(data);
			}
			else
			{
				L_CORE_ERROR("Skybox Texture Failed to Load: {0}", m_TextureFilePaths[i].string());
				stbi_image_free(data);
				return GL_FALSE;
			}
		}
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

		L_CORE_INFO("All Skybox Textures Loaded Successfully");
		return GL_TRUE;
	}

	/// <summary>
	/// Load individual texture for indiviaul skybox face.
	/// </summary>
	/// <param name="binding">This is the face of the cube map.</param>
	GLboolean SkyboxMaterial::LoadSkyboxTexture(const L_SKYBOX_BINDING& binding, const std::filesystem::path& filePath)
	{
		glBindTexture(GL_TEXTURE_CUBE_MAP, m_SkyboxID);

		int width, height, nrChannels;

		unsigned char* data = stbi_load(filePath.string().c_str(), &width, &height, &nrChannels, 0);
		if (data)
		{
			GLenum format = GL_RGBA;

			switch (nrChannels) {
			case 1:
				format = GL_RED;
				break;
			case 3:
				format = GL_RGB;
				break;
			case 4:
				format = GL_RGBA;
				break;
			}
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + binding, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
			stbi_image_free(data);
		}
		else
		{
			L_CORE_WARN("Skybox Texture Failed to Load: {0}", filePath.string());
			stbi_image_free(data);
			return GL_FALSE;
		}
		
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

		L_CORE_INFO("Skybox Texture Loaded Successfully: {0}", filePath.filename().replace_extension().string());
		return GL_TRUE;
	}

	void SkyboxMaterial::UpdateUniforms(const Camera& camera) {

		if (m_MaterialShader) {
			if (&camera != nullptr) {
				m_MaterialShader->SetMat4("u_VertexIn.Proj", camera.GetProjMatrix());
				glm::mat4 view = glm::mat4(glm::mat3(camera.GetViewMatrix()));
				m_MaterialShader->SetMat4("u_VertexIn.View", view);
			}
		}
		else {
			L_CORE_ERROR("Error Updating Uniforms - Shader Not Found for Skybox Material: {0}", m_MaterialName);
		}
	}

	void SkyboxMaterial::Serialize(const std::filesystem::path& path) {

		m_MaterialFilePath = path;

		YAML::Emitter materialOut;
		materialOut << YAML::BeginMap;
		materialOut << YAML::Key << "Material Name" << YAML::Value << m_MaterialName;
		materialOut << YAML::Key << "Material Type" << YAML::Value << "SkyboxMaterial";
		materialOut << YAML::Key << "TextureFilePaths" << YAML::Value << YAML::BeginMap;

		std::unordered_map<int, std::string> indexKeyMap{
			{ 0, "Right"},
			{ 1, "Left"},
			{ 2, "Top"},
			{ 3, "Bottom"},
			{ 4, "Back"},
			{ 5, "Front"},
		};

		std::filesystem::path asset_directory;
		if (auto project_ref = Project::GetActiveProject())
			asset_directory = project_ref->GetConfig().AssetDirectory;

		for (auto& pair : indexKeyMap) {
			materialOut <<
				YAML::Key << pair.second <<
				YAML::Value << std::filesystem::relative(m_TextureFilePaths[pair.first], asset_directory).string();
		}

		materialOut << YAML::EndMap << YAML::EndMap;

		std::filesystem::create_directories(m_MaterialFilePath.parent_path());

		std::ofstream fout(m_MaterialFilePath);
		fout << materialOut.c_str();
	}

	bool SkyboxMaterial::Deserialize(const std::filesystem::path& path)
	{
		if (path.extension() != ".lmaterial") {

			L_CORE_WARN("Incompatible Material File Extension");
			L_CORE_WARN("Extension Used: {0}", path.extension().string());
			L_CORE_WARN("Extension Expected: .lmaterial");
		}
		else
		{
			YAML::Node data;

			try {
				data = YAML::LoadFile(path.string());
			}
			catch (YAML::ParserException e) {
				L_CORE_ERROR("YAML-CPP Failed to Load Scene File: '{0}', {1}", path.string(), e.what());
				return false;
			}

			if (!data["Material Type"] || data["Material Type"].as<std::string>() != "SkyboxMaterial") {
				L_CORE_ERROR("Material Type Node is Not Skybox Material: '{0}'", path.string());
				return false;
			}

			if (!data["Material Name"]) {
				L_CORE_ERROR("Material Name Node Not Correctly Declared in File: '{0}'", path.string());
				return false;
			}

			if (!data["TextureFilePaths"]) {
				L_CORE_ERROR("TextureFilePaths Node Not Correctly Declared in File: '{0}'", path.string());
				return false;
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

				std::filesystem::path asset_directory;
				if (auto project_ref = Project::GetActiveProject())
					asset_directory = project_ref->GetConfig().AssetDirectory;

				auto texturesData = data["TextureFilePaths"];
				if (texturesData)
					for (auto& pair : indexKeyMap)
						if (texturesData[pair.second])
							textureFilePathArray[pair.first] = asset_directory / texturesData[pair.second].as<std::string>();

				LoadSkybox(textureFilePathArray);
				SetName(data["Material Name"].as<std::string>());
				SetMaterialFilePath(path);

				return true;
			}
		}
		return false;
	}

	GLboolean SkyboxMaterial::Bind() {
		if (m_MaterialShader) {
			m_MaterialShader->Bind();
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_CUBE_MAP, m_SkyboxID);
			return GL_TRUE;
		}

		L_CORE_ERROR("Shader Not Found for Material: {0}", this->GetName());
		return GL_FALSE;
	}
	void SkyboxMaterial::UnBind() {
		
		glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
		glActiveTexture(GL_TEXTURE0);
		glUseProgram(0);
	}

#pragma endregion

	SkyboxComponent::SkyboxComponent() {

		float skyboxVertices[] = {
			// positions          
			-1.0f,  1.0f, -1.0f,
			-1.0f, -1.0f, -1.0f,
			 1.0f, -1.0f, -1.0f,
			 1.0f, -1.0f, -1.0f,
			 1.0f,  1.0f, -1.0f,
			-1.0f,  1.0f, -1.0f,

			-1.0f, -1.0f,  1.0f,
			-1.0f, -1.0f, -1.0f,
			-1.0f,  1.0f, -1.0f,
			-1.0f,  1.0f, -1.0f,
			-1.0f,  1.0f,  1.0f,
			-1.0f, -1.0f,  1.0f,

			 1.0f, -1.0f, -1.0f,
			 1.0f, -1.0f,  1.0f,
			 1.0f,  1.0f,  1.0f,
			 1.0f,  1.0f,  1.0f,
			 1.0f,  1.0f, -1.0f,
			 1.0f, -1.0f, -1.0f,

			-1.0f, -1.0f,  1.0f,
			-1.0f,  1.0f,  1.0f,
			 1.0f,  1.0f,  1.0f,
			 1.0f,  1.0f,  1.0f,
			 1.0f, -1.0f,  1.0f,
			-1.0f, -1.0f,  1.0f,

			-1.0f,  1.0f, -1.0f,
			 1.0f,  1.0f, -1.0f,
			 1.0f,  1.0f,  1.0f,
			 1.0f,  1.0f,  1.0f,
			-1.0f,  1.0f,  1.0f,
			-1.0f,  1.0f, -1.0f,

			-1.0f, -1.0f, -1.0f,
			-1.0f, -1.0f,  1.0f,
			 1.0f, -1.0f, -1.0f,
			 1.0f, -1.0f, -1.0f,
			-1.0f, -1.0f,  1.0f,
			 1.0f, -1.0f,  1.0f
		};
				
		// Create Buffer
		VertexBuffer* vbo = new VertexBuffer(skyboxVertices, sizeof(skyboxVertices) / sizeof(float));
		vbo->SetLayout({ { ShaderDataType::Float3, "aPos" } });

		// Create Vertex Array and Assign Buffer
		m_VAO = std::make_shared<VertexArray>();
		m_VAO->AddVertexBuffer(vbo);
	}
}