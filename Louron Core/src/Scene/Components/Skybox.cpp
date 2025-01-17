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

#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include <stb_image/stb_image_resize2.h>

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
		ConstructSkyboxCubeMap(); 
	}

	SkyboxMaterial::~SkyboxMaterial() { glDeleteTextures(1, &m_SkyboxID); }

	/// <summary>
	/// Assign a new asset to a face of the skybox Cube Map. This will rebuild the skybox.
	/// </summary>
	/// <param name="binding">This is the face of the cube map.</param>
	void SkyboxMaterial::SetSkyboxFaceTexture(const L_SKYBOX_BINDING& binding, const AssetHandle& texture_asset_handle) {
		m_TextureAssetHandles[binding] = texture_asset_handle;
		ConstructSkyboxCubeMap();
	}

	void SkyboxMaterial::UpdateUniforms(Camera* camera) {

		if (m_MaterialShader) {

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_CUBE_MAP, m_SkyboxID);
			m_MaterialShader->SetInt("u_Skybox", 0);

			if (&camera) {
				m_MaterialShader->SetMat4("u_VertexIn.Proj", camera->GetProjMatrix());
				glm::mat4 view = glm::mat4(glm::mat3(camera->GetViewMatrix()));
				m_MaterialShader->SetMat4("u_VertexIn.View", view);
			}
		}
		else {
			L_CORE_ERROR("Error Updating Uniforms - Shader Not Found for Skybox Material.");
		}
	}

	/// <summary>
	/// This will serialise the data of the Skybox Material into a file.
	/// </summary>
	/// <param name="path">If you pass a value, it will serialise this into a folder INSIDE the asset directory. If empty, it will get the metadata of the Asset and serialise to that filepath.</param>
	void SkyboxMaterial::Serialize(const std::filesystem::path& path) {

		std::filesystem::path out_path;

		const AssetMetaData& meta_data = Project::GetStaticEditorAssetManager()->GetMetadata(Handle);

		if (path.empty())
			out_path = Project::GetActiveProject()->GetProjectDirectory() / Project::GetActiveProject()->GetConfig().AssetDirectory / meta_data.FilePath;
		else
			out_path = Project::GetActiveProject()->GetProjectDirectory() / Project::GetActiveProject()->GetConfig().AssetDirectory / path;

		YAML::Emitter out_file;
		out_file << YAML::BeginMap;
		std::string asset_name = (meta_data.AssetName.empty()) ? path.filename().replace_extension().string() : meta_data.AssetName;
		out_file << YAML::Key << "Material Asset Name" << YAML::Value << asset_name;
		out_file << YAML::Key << "Material Asset Type" << YAML::Value << AssetTypeToString(AssetType::Material_Skybox);
		out_file << YAML::Key << "Skybox Asset Handles" << YAML::Value;

		{
			out_file << YAML::BeginSeq;
			for (const auto& handle : m_TextureAssetHandles) {
				out_file << (uint32_t)handle;
			}
			out_file << YAML::EndSeq;
		}

		out_file << YAML::EndMap;

		std::filesystem::create_directories(out_path.parent_path());

		std::ofstream fout(out_path);
		fout << out_file.c_str();
	}

	bool SkyboxMaterial::Deserialize(const std::filesystem::path& path)
	{
		std::filesystem::path in_path;

		const AssetMetaData& meta_data = Project::GetStaticEditorAssetManager()->GetMetadata(Handle);

		if (path.empty()) {
			in_path = Project::GetActiveProject()->GetConfig().AssetDirectory / meta_data.FilePath;
		}
		else {
			in_path = Project::GetActiveProject()->GetConfig().AssetDirectory / path;
		}

		std::ifstream fin(in_path);
		if (!fin.is_open()) {
			L_CORE_ERROR("Failed to Open File for Reading: {0}", in_path.string().c_str());
			return false;
		}

		YAML::Node node = YAML::Load(fin);

		if (!node["Material Asset Name"]) {
			L_CORE_ERROR("Invalid YAML Format: Missing 'Material Asset Name'");
			return false;
		}

		if (node["Material Asset Type"].as<std::string>() != AssetTypeToString(AssetType::Material_Skybox)) {
			L_CORE_ERROR("Invalid Material Type: Expected '{0}'", AssetTypeToString(AssetType::Material_Skybox));
			return false;
		}

		int i = 0;
		for (const auto& handleNode : node["Skybox Asset Handles"]) {
			m_TextureAssetHandles[i] = handleNode.as<uint32_t>();
			i++;
		}

		ConstructSkyboxCubeMap();
		return true;
	}

	GLboolean SkyboxMaterial::Bind() {

		if (m_MaterialShader) {
			m_MaterialShader->Bind();
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

	void SkyboxMaterial::ConstructSkyboxCubeMap(){

		// 1. Ensure that all textures are the same size
		// 2. Load data for all textures using STB_IMAGE
		// 2.a. if the texture asset is not valid, or cannot be loaded, we need to create data of a blank texture that OpenGL can use
		// 2.b. we need to stretch/shrink all textures to an appropriate size, let's use 1024x1024 for now
		// 3. Build GL_TEXTURE_CUBE_MAP
		// 4. Set opengl texture parameters
		// 5. Release data for all textures

		glDeleteTextures(1, &m_SkyboxID);
		m_SkyboxID = -1;
		glGenTextures(1, &m_SkyboxID);

		glBindTexture(GL_TEXTURE_CUBE_MAP, m_SkyboxID);

		const std::filesystem::path asset_directory = Project::GetActiveProject()->GetConfig().AssetDirectory;
		const int desired_size = 1024; // Desired size for all faces

		for (int i = 0; i < m_TextureAssetHandles.size(); i++) {
			bool texture_valid = false; // Use this to check if the texture has been successfully loaded this iteration of the cube_map build

			if (Project::GetStaticEditorAssetManager()->IsAssetHandleValid(m_TextureAssetHandles[i])) {
				int width, height, nrChannels;
				const std::filesystem::path& texture_file_path = Project::GetActiveProject()->GetProjectDirectory() / asset_directory / Project::GetStaticEditorAssetManager()->GetMetadata(m_TextureAssetHandles[i]).FilePath;
				unsigned char* data = stbi_load(texture_file_path.string().c_str(), &width, &height, &nrChannels, 0);

				if (data) {
					texture_valid = true;
					GLenum format = (nrChannels == 1) ? GL_RED : (nrChannels == 3) ? GL_RGB : GL_RGBA;

					if (width != desired_size || height != desired_size)
					{
						unsigned char* resized_data = new unsigned char[desired_size * desired_size * nrChannels];

						stbir_pixel_layout pixel_format = format == GL_RGB ? stbir_pixel_layout::STBIR_RGB : stbir_pixel_layout::STBIR_RGBA;
						stbir_resize_uint8_srgb(data, width, height, 0, resized_data, desired_size, desired_size, 0, pixel_format);
						glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA, desired_size, desired_size, 0, format, GL_UNSIGNED_BYTE, resized_data);

						delete[] resized_data;
					}
					else {
						glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA, desired_size, desired_size, 0, format, GL_UNSIGNED_BYTE, data);
					}

					stbi_image_free(data);
				}
				else {
					L_CORE_WARN("Skybox Texture Failed to Load: {0}", texture_file_path.string().c_str());
					stbi_image_free(data);
				}
			}

			if (!texture_valid) {
				std::vector<GLubyte> texture_data(desired_size * desired_size * 4, 255);
				glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA, desired_size, desired_size, 0, GL_RGBA, GL_UNSIGNED_BYTE, texture_data.data());
			}

			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		}

		L_CORE_INFO("Skybox Texture Loaded Successfully.");
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

	void SkyboxComponent::Serialize(YAML::Emitter& out) {

		out << YAML::Key << "SkyboxComponent";
		out << YAML::BeginMap;

		out << YAML::Key << "MaterialAssetHandle" << YAML::Value << (uint32_t)SkyboxMaterialAssetHandle;

		if (auto material_ref = Project::GetStaticEditorAssetManager()->GetAsset<SkyboxMaterial>(SkyboxMaterialAssetHandle); material_ref) {

			const AssetMetaData& meta_data = Project::GetStaticEditorAssetManager()->GetMetadata(SkyboxMaterialAssetHandle);
			material_ref->Serialize(meta_data.FilePath);

			L_CORE_INFO("Skybox Material ({0}) Saved At: {1}", meta_data.FilePath.filename().replace_extension().string().c_str(), meta_data.FilePath.string().c_str());

		}
		out << YAML::EndMap;
	}

	bool SkyboxComponent::Deserialize(const YAML::Node data)
	{

		YAML::Node component = data;

		if (component["MaterialAssetHandle"]) {
			SkyboxMaterialAssetHandle = component["MaterialAssetHandle"].as<uint32_t>();
		}
		else {
			return false;
		}

		auto material_ref = Project::GetStaticEditorAssetManager()->GetAsset<SkyboxMaterial>(SkyboxMaterialAssetHandle);
		if (material_ref) {
			const AssetMetaData& meta_data = Project::GetStaticEditorAssetManager()->GetMetadata(SkyboxMaterialAssetHandle);
			std::filesystem::path material_path = Project::GetActiveProject()->GetConfig().AssetDirectory / meta_data.FilePath;

			try {
				material_ref->Deserialize(Project::GetActiveProject()->GetProjectDirectory() / "Assets" / meta_data.FilePath);
			}
			catch (const std::exception& e) {
				L_CORE_ERROR("Failed to deserialize Skybox Material from {0}: {1}", material_path.string(), e.what());
				return false;
			}
		}

		return true;
	}
}