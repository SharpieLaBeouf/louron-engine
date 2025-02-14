#include "Skybox.h"

// Louron Core Headers
#include "../../Debug/Assert.h"

#include "../../Asset/Asset Manager API.h"
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
	SkyboxMaterial::SkyboxMaterial() 
	{
		SetShader(AssetManager::GetInbuiltShader("Skybox")->Handle);

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

	void SkyboxMaterial::UpdateUniforms(std::shared_ptr<MaterialUniformBlock> custom_uniform_block) {

		if (auto shader_ref = GetShader(); shader_ref)
		{
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_CUBE_MAP, m_SkyboxID);
			shader_ref->SetInt("u_Skybox", 0);
		}
		else 
		{
			L_CORE_ERROR("Error Updating Uniforms - Shader Not Found for Skybox Material.");
		}
	}

	/// <summary>
	/// This will serialise the data of the Skybox Material into a file.
	/// </summary>
	/// <param name="path">If you pass a value, it will serialise this into a folder INSIDE the asset directory. If empty, it will get the metadata of the Asset and serialise to that filepath.</param>
	void SkyboxMaterial::Serialize(YAML::Emitter& out) const
	{
		out << YAML::Key << "Material Asset Name" << YAML::Value << GetName();
		out << YAML::Key << "Material Asset Type" << YAML::Value << AssetUtils::AssetTypeToString(AssetType::Material_Skybox);
		out << YAML::Key << "Skybox Asset Handles" << YAML::Value;

		{
			out << YAML::BeginSeq;
			for (const auto& handle : m_TextureAssetHandles) {
				out << (uint32_t)handle;
			}
			out << YAML::EndSeq;
		}
	}

	bool SkyboxMaterial::Deserialize(const std::filesystem::path& path)
	{
		std::filesystem::path in_path;

		if (path.empty()) {
			const AssetMetaData& meta_data = Project::GetStaticEditorAssetManager()->GetMetadata(Handle);
			in_path = Project::GetActiveProject()->GetAssetDirectory() / meta_data.FilePath;
		}
		else 
		{
			in_path = path;
		}

		std::ifstream fin(in_path);
		if (!fin.is_open()) 
		{
			L_CORE_ERROR("Failed to Open File for Reading: {0}", in_path.string().c_str());
			return false;
		}

		YAML::Node node = YAML::Load(fin);

		if (!node["Material Asset Name"]) 
		{
			L_CORE_ERROR("Invalid YAML Format: Missing 'Material Asset Name'");
			return false;
		}

		if (node["Material Asset Type"].as<std::string>() != AssetUtils::AssetTypeToString(AssetType::Material_Skybox)) 
		{
			L_CORE_ERROR("Invalid Material Type: Expected '{0}'", AssetUtils::AssetTypeToString(AssetType::Material_Skybox));
			return false;
		}

		std::string name = node["Material Asset Name"].as<std::string>();
		SetName(name);

		int i = 0;
		for (const auto& handleNode : node["Skybox Asset Handles"]) 
		{
			m_TextureAssetHandles[i] = handleNode.as<uint32_t>();
			i++;
		}

		ConstructSkyboxCubeMap();
		return true;
	}

	bool SkyboxMaterial::Bind() const
	{
		if (auto shader_ref = GetShader(); shader_ref) {
			shader_ref->Bind();
			return true;
		}

		L_CORE_ERROR("Shader Not Found for Material: {0}", this->GetName());
		return false;
	}

	void SkyboxMaterial::UnBind() const
	{
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

		for (int i = 0; i < m_TextureAssetHandles.size(); i++) 
		{
			bool texture_valid = false; // Use this to check if the texture has been successfully loaded this iteration of the cube_map build

			if (AssetManager::IsAssetHandleValid(m_TextureAssetHandles[i])) 
			{
				int width, height, nrChannels;
				const std::filesystem::path& texture_file_path = Project::GetActiveProject()->GetAssetDirectory() / Project::GetStaticEditorAssetManager()->GetMetadata(m_TextureAssetHandles[i]).FilePath;
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
					else 
					{
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

		static float skyboxVertices[] = {
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

	void SkyboxComponent::Serialize(YAML::Emitter& out) const
	{

		out << YAML::Key << "SkyboxComponent";
		out << YAML::BeginMap;

		out << YAML::Key << "MaterialAssetHandle" << YAML::Value << (uint32_t)SkyboxMaterialAssetHandle;

		out << YAML::EndMap;
	}

	bool SkyboxComponent::Deserialize(const YAML::Node data)
	{
		YAML::Node component = data;

		if (!component["MaterialAssetHandle"])
			return false;

		SkyboxMaterialAssetHandle = component["MaterialAssetHandle"].as<uint32_t>();
		return true;
	}

}