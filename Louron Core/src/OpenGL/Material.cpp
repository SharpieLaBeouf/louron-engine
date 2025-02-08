#include "Material.h"

// Louron Core Headers
#include "../Core/Engine.h"

#include "../Debug/Assert.h"

#include "../Asset/Asset Manager API.h"

// C++ Standard Library Headers

// External Vendor Library Headers
#ifndef YAML_CPP_STATIC_DEFINE
#define YAML_CPP_STATIC_DEFINE
#endif
#include <yaml-cpp/yaml.h>

namespace Louron {

#pragma region Base Class Material

	GLboolean Material::Bind() {
		if (auto shader = AssetManager::GetAsset<Shader>(m_ShaderAssetHandle); shader) {
			shader->Bind();
			return GL_TRUE;
		}
		L_CORE_ERROR("Shader Not Found for Material: {0}", GetName());
		return GL_FALSE;
	}

	void Material::UnBind() {

		for (int i = 0; i < m_Textures.size(); i++) {
			if (m_Textures[i]) {
				glActiveTexture(GL_TEXTURE0 + i);
				glBindTexture(GL_TEXTURE_2D, 0);
			}
		}
		glActiveTexture(GL_TEXTURE0); 
		glUseProgram(0);
	}

	void Material::UpdateUniforms(const glm::vec3& camera_position, const glm::mat4& projection_matrix, const glm::mat4& view_matrix, std::shared_ptr<MaterialUniformBlock> custom_uniform_block) {

		if (auto shader = AssetManager::GetAsset<Shader>(m_ShaderAssetHandle); shader) {
			shader->SetFloat("u_Material.shine", m_Shine);
			shader->SetFloatVec4("u_Material.diffuse", m_Diffuse);
			shader->SetFloatVec4("u_Material.specular", m_Specular);

			for (int i = 0; i < L20_TOTAL_ELEMENTS; i++) {

				shader->SetInt(std::string("u_Material." + m_TextureUniformNames[i]).c_str(), i);
				glActiveTexture(GL_TEXTURE0 + i);
				glBindTexture(GL_TEXTURE_2D, m_Textures[i]->GetID());
			}

			shader->SetMat4("u_VertexIn.Proj", projection_matrix);
			shader->SetMat4("u_VertexIn.View", view_matrix);
			shader->SetFloatVec3("u_CameraPos", camera_position);
		}
		else {
			L_CORE_ERROR("Shader Not Found for Material: {0}", GetName());
		}
	}

	void Material::SetShader(AssetHandle shader_handle) {
		m_ShaderAssetHandle = shader_handle;
	}
	void Material::SetTexture(std::shared_ptr<Texture> texture, TextureMapType textureType) {
		m_Textures[textureType] = texture; 
	}

	std::shared_ptr<Shader> Material::GetShader() {
		return AssetManager::GetAsset<Shader>(m_ShaderAssetHandle);
	}

	AssetHandle Material::GetShaderHandle()
	{
		return m_ShaderAssetHandle;
	}

	/// <summary>
	/// DEFAULT CONSTRUCTOR - Initialise Material with NO SHADER and BLANK TEXTURE
	/// </summary>
	Material::Material() : m_ShaderAssetHandle(AssetManager::GetInbuiltShader("Default Shader")->Handle) {
		for (int i = 0; i < TextureMapType::L20_TOTAL_ELEMENTS; i++)
			m_Textures[i] = Engine::Get().GetTextureLibrary().GetDefaultTexture();
	}

	/// <summary>
	/// Initialise Material with BLANK TEXTURE and CUSTOM NAME
	/// </summary>
	/// <param name="name"></param>
	/// <param name="shader"></param>
	Material::Material(const std::string& name, std::shared_ptr<Shader> shader) : m_Name(name), m_ShaderAssetHandle(shader->Handle) {
		for (int i = 0; i < TextureMapType::L20_TOTAL_ELEMENTS; i++)
			m_Textures[i] = Engine::Get().GetTextureLibrary().GetDefaultTexture();
	}

	/// <summary>
	/// Initialise Material with BLANK TEXTURE
	/// </summary>
	Material::Material(AssetHandle shader) : m_ShaderAssetHandle(shader) {
		for (int i = 0; i < TextureMapType::L20_TOTAL_ELEMENTS; i++)
			m_Textures[i] = Engine::Get().GetTextureLibrary().GetDefaultTexture();
	}

	/// <summary>
	/// Initialise Material with NO SHADER and TEXTURE PARAMETER
	/// </summary>
	Material::Material(std::shared_ptr<Texture> texture) : m_ShaderAssetHandle(AssetManager::GetInbuiltShader("Default Shader")->Handle) {
		for (int i = 0; i < TextureMapType::L20_TOTAL_ELEMENTS; i++)
			m_Textures[i] = texture;
	}

	/// <summary>
	/// Initialise Material with SHADER and TEXTURE PARAMETER
	/// </summary>
	Material::Material(AssetHandle shader, std::shared_ptr<Texture> texture) : m_ShaderAssetHandle(shader) {
		for (int i = 0; i < TextureMapType::L20_TOTAL_ELEMENTS; i++)
			m_Textures[i] = texture;
	}

	/// <summary>
	/// Initialise Material with SHADER and TEXTURE UNORDERED MAP PARAMETER
	/// </summary>
	Material::Material(AssetHandle shader, std::unordered_map<GLint, std::shared_ptr<Texture>>& textures) : m_ShaderAssetHandle(shader) {
		for (int i = 0; i < textures.size(); i++)
			m_Textures[i] = textures[i];
	}

	float Material::GetShine() const { return m_Shine; }
	glm::vec4* Material::GetDiffuse() { return &m_Diffuse; }
	glm::vec4* Material::GetSpecular() { return &m_Specular; }

	void Material::SetShine(float shine) { m_Shine = shine; }
	void Material::SetDiffuse(const glm::vec4& val) { m_Diffuse = val; }
	void Material::SetSpecular(const glm::vec4& val) { m_Specular = val; }

	void Material::AddTextureMap(GLint type, std::shared_ptr<Texture> val) {
		if (auto shader = AssetManager::GetAsset<Shader>(m_ShaderAssetHandle); shader) {
			shader->Bind();
			shader->SetInt(std::string("u_Material." + m_TextureUniformNames[type]).c_str(), type);
			shader->UnBind();
		}
		else
			L_CORE_WARN("Shader Not Linked to Material - Cannot Set Texture Unit"); 

		if (type < m_Textures.size() && type > -1) {
			m_Textures[type] = val;
		}
		else
			L_CORE_WARN("Texture Slot Out of Bounds"); 
	}

	std::shared_ptr<Texture> Material::GetTextureMap(GLint type) {
		return m_Textures[type];
	}

#pragma endregion

#pragma region PBR Materials

	PBRMaterial::PBRMaterial()
	{
		SetShader(AssetManager::GetInbuiltShader("FP_Material_PBR_Shader")->Handle);
	}

	PBRMaterial::PBRMaterial(AssetHandle shader_handle)
	{
		SetShader(shader_handle);
	}

	GLboolean PBRMaterial::Bind()  {

		if(auto shader_ref = AssetManager::GetAsset<Shader>(m_ShaderAssetHandle); shader_ref) {
			shader_ref->Bind();
			return GL_TRUE;
		}
		L_CORE_ERROR("Shader Not Found for PBR Material: {0}", m_MaterialName);
		return GL_FALSE;
	}

	void PBRMaterial::UnBind() {

		for (int i = 0; i < 3; i++) {
			glActiveTexture(GL_TEXTURE0 + i);
			glBindTexture(GL_TEXTURE_2D, 0);
		}

		glActiveTexture(GL_TEXTURE0);
		glUseProgram(0);
	}

	void PBRMaterial::UpdateUniforms(const glm::vec3& camera_position, const glm::mat4& projection_matrix, const glm::mat4& view_matrix, std::shared_ptr<MaterialUniformBlock> custom_uniform_block) {

		if (auto shader_ref = AssetManager::GetAsset<Shader>(m_ShaderAssetHandle); shader_ref) {

			shader_ref->SetFloatVec4("u_Material.albedoTint", m_AlbedoTint);
			shader_ref->SetFloat("u_Material.roughness", m_Roughness);
			shader_ref->SetFloat("u_Material.metallicScale", IsMetallicTextureSet() ? 1.0f : m_MetallicScale);

			GLint maxTextureUnits = 3;
			glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &maxTextureUnits);

			GLint texture_unit = 0;
			if (auto texture_ref = (m_AlbedoTexture == NULL_UUID) ? Engine::Get().GetTextureLibrary().GetDefaultTexture() : AssetManager::GetAsset<Texture>(m_AlbedoTexture); texture_ref && *texture_ref && texture_unit < maxTextureUnits) {
				glActiveTexture(GL_TEXTURE0 + texture_unit);
				shader_ref->SetInt("u_Material.albedoTexture", texture_unit);
				texture_ref->Bind();
				texture_unit++;
			}

			if (auto texture_ref = (m_MetallicTexture == NULL_UUID) ? Engine::Get().GetTextureLibrary().GetDefaultTexture() : AssetManager::GetAsset<Texture>(m_MetallicTexture); texture_ref && *texture_ref && texture_unit < maxTextureUnits) {
				glActiveTexture(GL_TEXTURE0 + texture_unit);
				shader_ref->SetInt("u_Material.metallicTexture", texture_unit);
				texture_ref->Bind();
				texture_unit++;
			}

			if (auto texture_ref = (m_NormalTexture == NULL_UUID) ? Engine::Get().GetTextureLibrary().GetDefaultNormalTexture() : AssetManager::GetAsset<Texture>(m_NormalTexture); texture_ref && *texture_ref && texture_unit < maxTextureUnits) {
				glActiveTexture(GL_TEXTURE0 + texture_unit);
				shader_ref->SetInt("u_Material.normalTexture", texture_unit);
				texture_ref->Bind();
				texture_unit++;
			}

			shader_ref->SetMat4("u_VertexIn.Proj", projection_matrix);
			shader_ref->SetMat4("u_VertexIn.View", view_matrix);
			shader_ref->SetFloatVec3("u_CameraPos", camera_position);

			if (!m_UniformBlock)
				return;

			// All other texture units are pre-dedicated to other things such as depth map and shadow maps. 
			texture_unit = 7;

			const auto& uniforms = (custom_uniform_block) ? custom_uniform_block->GetUniforms() : m_UniformBlock->GetUniforms();
			for (const auto& uniform : uniforms)
			{
				std::string name = "u_MaterialUniforms." + uniform.first;
				const GLSLType& type = uniform.second.first;
				const UniformValue& value = uniform.second.second;

				switch (type)

				{
					case GLSLType::Bool:	shader_ref->SetBool(name.c_str(), std::get<bool>(value)); break;
					case GLSLType::BVec2:	shader_ref->SetBoolVec2(name.c_str(), std::get<glm::bvec2>(value)); break;
					case GLSLType::BVec3:	shader_ref->SetBoolVec3(name.c_str(), std::get<glm::bvec3>(value)); break;
					case GLSLType::BVec4:	shader_ref->SetBoolVec4(name.c_str(), std::get<glm::bvec4>(value)); break;

					case GLSLType::Int:		shader_ref->SetInt(name.c_str(), std::get<int>(value)); break;
					case GLSLType::IVec2:	shader_ref->SetIntVec2(name.c_str(), std::get<glm::ivec2>(value)); break;
					case GLSLType::IVec3:	shader_ref->SetIntVec3(name.c_str(), std::get<glm::ivec3>(value)); break;
					case GLSLType::IVec4:	shader_ref->SetIntVec4(name.c_str(), std::get<glm::ivec4>(value)); break;

					case GLSLType::Uint:	shader_ref->SetUInt(name.c_str(), std::get<GLuint>(value)); break;
					case GLSLType::UVec2:	shader_ref->SetUIntVec2(name.c_str(), std::get<glm::uvec2>(value)); break;
					case GLSLType::UVec3:	shader_ref->SetUIntVec3(name.c_str(), std::get<glm::uvec3>(value)); break;
					case GLSLType::UVec4:	shader_ref->SetUIntVec4(name.c_str(), std::get<glm::uvec4>(value)); break;

					case GLSLType::Float:	shader_ref->SetFloat(name.c_str(), std::get<float>(value)); break;
					case GLSLType::Vec2:	shader_ref->SetFloatVec2(name.c_str(), std::get<glm::vec2>(value)); break;
					case GLSLType::Vec3:	shader_ref->SetFloatVec3(name.c_str(), std::get<glm::vec3>(value)); break;
					case GLSLType::Vec4:	shader_ref->SetFloatVec4(name.c_str(), std::get<glm::vec4>(value)); break;

					case GLSLType::Double:	shader_ref->SetDouble(name.c_str(), std::get<double>(value)); break;
					case GLSLType::DVec2:	shader_ref->SetDoubleVec2(name.c_str(), std::get<glm::dvec2>(value)); break;
					case GLSLType::DVec3:	shader_ref->SetDoubleVec3(name.c_str(), std::get<glm::dvec3>(value)); break;
					case GLSLType::DVec4:	shader_ref->SetDoubleVec4(name.c_str(), std::get<glm::dvec4>(value)); break;

					case GLSLType::Mat2:	shader_ref->SetMat2(name.c_str(), std::get<glm::mat2>(value)); break;
					case GLSLType::Mat3:	shader_ref->SetMat3(name.c_str(), std::get<glm::mat3>(value)); break;
					case GLSLType::Mat4:	shader_ref->SetMat4(name.c_str(), std::get<glm::mat4>(value)); break;

					case GLSLType::Sampler2D:
					case GLSLType::Sampler2DShadow:
					{
						AssetHandle texture_handle = std::get<AssetHandle>(value);

						if (auto texture_ref = AssetManager::GetAsset<Texture>(texture_handle); texture_ref && *texture_ref && texture_unit < maxTextureUnits) {
							glActiveTexture(GL_TEXTURE0 + texture_unit);
							shader_ref->SetInt(name.c_str(), texture_unit);
							texture_ref->Bind();
							texture_unit++;
						}
						else
						{
							glActiveTexture(GL_TEXTURE0 + texture_unit);
							shader_ref->SetInt(name.c_str(), texture_unit);
							Engine::Get().GetTextureLibrary().GetDefaultTexture()->Bind();
							texture_unit++;
						}


						break;
					}

					case GLSLType::Sampler1D:
					case GLSLType::Sampler1DShadow:
					case GLSLType::Sampler1DArray:
					case GLSLType::Sampler1DArrayShadow:
					case GLSLType::Sampler2DArray:
					case GLSLType::Sampler2DArrayShadow:
					case GLSLType::Sampler3D:
					case GLSLType::SamplerCube:
					case GLSLType::SamplerCubeArray:
					case GLSLType::SamplerCubeShadow:
					case GLSLType::SamplerCubeArrayShadow:
					default:
						// Handle unknown type case
						break;
				}

			}

		}
		else {
			L_CORE_ERROR("Shader Not Found for PBR Material: {0}", GetName());
		}

	}

	bool PBRMaterial::IsAlbedoTextureSet() const { return m_AlbedoTexture != NULL_UUID; }
	bool PBRMaterial::IsMetallicTextureSet() const { return m_MetallicTexture != NULL_UUID; }
	bool PBRMaterial::IsNormalTextureSet() const { return m_NormalTexture != NULL_UUID; }

	#pragma region Getters and Setters

	void PBRMaterial::SetAlbedoTexture(AssetHandle texture) { m_AlbedoTexture = texture; }
	void PBRMaterial::SetMetallicTexture(AssetHandle texture) { m_MetallicTexture = texture; }
	void PBRMaterial::SetNormalTexture(AssetHandle texture) { m_NormalTexture = texture; }

	void PBRMaterial::SetRoughness(float roughness) { m_Roughness = roughness; }
	void PBRMaterial::SetMetallic(float metallic) { m_MetallicScale = metallic; }
	void PBRMaterial::SetAlbedoTintColour(const glm::vec4& albedo_colour) { m_AlbedoTint = albedo_colour; }

	void PBRMaterial::SetName(const std::string& name) { m_MaterialName = name; }
	void PBRMaterial::SetShader(AssetHandle shader_handle) 
	{
		if (shader_handle == NULL_UUID)
			return;
		
		if (auto shader = AssetManager::GetAsset<Shader>(shader_handle); shader)
		{
			if (m_UniformBlock)
			{
				m_UniformBlock.reset();
				m_UniformBlock = nullptr;
			}

			m_ShaderAssetHandle = shader_handle;
			m_UniformBlock = std::make_shared<MaterialUniformBlock>();
			m_UniformBlock->InitialiseFromShader(shader);
		}

	}

	AssetHandle PBRMaterial::GetAlbedoTextureAssetHandle() const { return m_AlbedoTexture; }

	AssetHandle PBRMaterial::GetMetallicTextureAssetHandle() const { return m_MetallicTexture; }

	AssetHandle PBRMaterial::GetNormalTextureAssetHandle() const { return m_NormalTexture; }

	float PBRMaterial::GetRoughness() const { return m_Roughness; }
	float PBRMaterial::GetMetallic() const { return m_MetallicScale; }
	const glm::vec4& PBRMaterial::GetAlbedoTintColour() const { return m_AlbedoTint; }

	std::shared_ptr<Shader> PBRMaterial::GetShader() {
		return AssetManager::GetAsset<Shader>(m_ShaderAssetHandle);
	}

	AssetHandle PBRMaterial::GetShaderHandle()
	{
		return m_ShaderAssetHandle;
	}

	const std::string& PBRMaterial::GetName() const { return m_MaterialName; }

	void PBRMaterial::Serialize(YAML::Emitter& out)
	{
		out << YAML::Key << "Material Asset Name" << YAML::Value << m_MaterialName;
		out << YAML::Key << "Material Asset Type" << YAML::Value << "AssetType::Material_Standard";
		out << YAML::Key << "Shader Handle" << YAML::Value << m_ShaderAssetHandle;
		out << YAML::Key << "Render Type" << YAML::Value << RenderTypeToString(m_RenderType);

		out << YAML::Key << "Roughness" << YAML::Value << m_Roughness;
		out << YAML::Key << "Metallic" << YAML::Value << m_MetallicScale;


		out << YAML::Key << "Albedo Tint" << YAML::Value << YAML::Flow
			<< YAML::BeginSeq
			<< m_AlbedoTint.r
			<< m_AlbedoTint.g
			<< m_AlbedoTint.b
			<< m_AlbedoTint.a
			<< YAML::EndSeq;

		out << YAML::Key << "AlbedoTextureAsset" << YAML::Value << m_AlbedoTexture;
		out << YAML::Key << "MetallicTextureAsset" << YAML::Value << m_MetallicTexture;
		out << YAML::Key << "NormalTextureAsset" << YAML::Value << m_NormalTexture;

		if (m_UniformBlock)
			m_UniformBlock->Serialize(out);
	}

	bool PBRMaterial::Deserialize(const std::filesystem::path& path)
	{
		YAML::Node data;

		if (!std::filesystem::exists(path))
			return false;

		try {
			data = YAML::LoadFile(path.string());
		}
		catch (YAML::ParserException e) {
			L_CORE_ERROR("YAML-CPP Failed to Load Scene File: '{0}', {1}", path.string(), e.what());
			return false;
		}

		if (!data)
			return false;

		if (data["Material Asset Name"]) {
			m_MaterialName = data["Material Asset Name"].as<std::string>();
		}

		if (data["Shader Handle"]) {
			SetShader(data["Shader Handle"].as<uint32_t>());
		}

		if (data["Render Type"]) {
			m_RenderType = StringToRenderType(data["Render Type"].as<std::string>());
		}

		if (data["Roughness"]) {
			m_Roughness = data["Roughness"].as<float>();
		}

		if (data["Metallic"]) {
			m_MetallicScale = data["Metallic"].as<float>();
		}

		if (data["Albedo Tint"]) {
			auto positionSeq = data["Albedo Tint"];
			if (positionSeq.IsSequence() && positionSeq.size() == 4) {
				m_AlbedoTint.r = positionSeq[0].as<float>();
				m_AlbedoTint.g = positionSeq[1].as<float>();
				m_AlbedoTint.b = positionSeq[2].as<float>();
				m_AlbedoTint.a = positionSeq[3].as<float>();
			}
		}

		if (data["AlbedoTextureAsset"]) {
			m_AlbedoTexture = data["AlbedoTextureAsset"].as<uint32_t>();
		}

		if (data["MetallicTextureAsset"]) {
			m_MetallicTexture = data["MetallicTextureAsset"].as<uint32_t>();
		}

		if (data["NormalTextureAsset"]) {
			m_NormalTexture = data["NormalTextureAsset"].as<uint32_t>();
		}

		if (data["Material Uniform Block"])
		{
			if (!m_UniformBlock)
			{
				m_UniformBlock = std::make_shared<MaterialUniformBlock>();
				m_UniformBlock->InitialiseFromShader(AssetManager::GetAsset<Shader>(m_ShaderAssetHandle));
			}

			m_UniformBlock->Deserialize(data["Material Uniform Block"]);
		}

		return true;
	}

	#pragma endregion

#pragma endregion

	MaterialUniformBlock::MaterialUniformBlock(const MaterialUniformBlock& other)
	{
		m_Uniforms = other.m_Uniforms;
	}

	MaterialUniformBlock::MaterialUniformBlock(MaterialUniformBlock&& other)
	{
		m_Uniforms = other.m_Uniforms; other.m_Uniforms = {};
	}

	void MaterialUniformBlock::InitialiseFromShader(std::shared_ptr<Shader> shader)
	{
		if (!shader)
			return;

		for (const auto& uniform : shader->GetCustomUniforms()) 
		{
			switch (uniform.type) {

				// Bool
				case GLSLType::Bool:    SetUniform(uniform.name, GLSLType::Bool, false);				break;
				case GLSLType::BVec2:   SetUniform(uniform.name, GLSLType::BVec2, glm::bvec2(false));	break;
				case GLSLType::BVec3:   SetUniform(uniform.name, GLSLType::BVec3, glm::bvec3(false));	break;
				case GLSLType::BVec4:   SetUniform(uniform.name, GLSLType::BVec4, glm::bvec4(false));	break;

				// Int
				case GLSLType::Int:     SetUniform(uniform.name, GLSLType::Int,	0);					break;
				case GLSLType::IVec2:   SetUniform(uniform.name, GLSLType::IVec2,  glm::ivec2(0));		break;
				case GLSLType::IVec3:   SetUniform(uniform.name, GLSLType::IVec3,  glm::ivec3(0));		break;
				case GLSLType::IVec4:   SetUniform(uniform.name, GLSLType::IVec4,  glm::ivec4(0));		break;

				// Unsigned Int
				case GLSLType::Uint:    SetUniform(uniform.name, GLSLType::Uint,	0u);				break;
				case GLSLType::UVec2:   SetUniform(uniform.name, GLSLType::UVec2,  glm::uvec2(0u));	break;
				case GLSLType::UVec3:   SetUniform(uniform.name, GLSLType::UVec3,  glm::uvec3(0u));	break;
				case GLSLType::UVec4:   SetUniform(uniform.name, GLSLType::UVec4,  glm::uvec4(0u));	break;

				// Float
				case GLSLType::Float:   SetUniform(uniform.name, GLSLType::Float,	0.0f);				break;
				case GLSLType::Vec2:    SetUniform(uniform.name, GLSLType::Vec2,	glm::vec2(0.0f));	break;
				case GLSLType::Vec3:    SetUniform(uniform.name, GLSLType::Vec3,	glm::vec3(0.0f));	break;
				case GLSLType::Vec4:    SetUniform(uniform.name, GLSLType::Vec4,	glm::vec4(0.0f));	break;

				// Double
				case GLSLType::Double:  SetUniform(uniform.name, GLSLType::Double, 0.0);				break;
				case GLSLType::DVec2:   SetUniform(uniform.name, GLSLType::DVec2,	glm::dvec2(0.0));	break;
				case GLSLType::DVec3:   SetUniform(uniform.name, GLSLType::DVec3,	glm::dvec3(0.0));	break;
				case GLSLType::DVec4:   SetUniform(uniform.name, GLSLType::DVec4,	glm::dvec4(0.0));	break;

				// Matrix
				case GLSLType::Mat2:    SetUniform(uniform.name, GLSLType::Mat2,	glm::mat2(1.0f));	break;
				case GLSLType::Mat3:    SetUniform(uniform.name, GLSLType::Mat3,	glm::mat3(1.0f));	break;
				case GLSLType::Mat4:    SetUniform(uniform.name, GLSLType::Mat4,	glm::mat4(1.0f));	break;

				// Textures - stored as AssetHandles
				case GLSLType::Sampler1D:
				case GLSLType::Sampler1DArray:
				case GLSLType::Sampler1DShadow:
				case GLSLType::Sampler1DArrayShadow:

				case GLSLType::Sampler2D:
				case GLSLType::Sampler2DArray:
				case GLSLType::Sampler2DShadow:
				case GLSLType::Sampler2DArrayShadow:

				case GLSLType::Sampler3D:

				case GLSLType::SamplerCube:
				case GLSLType::SamplerCubeArray:
				case GLSLType::SamplerCubeShadow:
				case GLSLType::SamplerCubeArrayShadow: SetUniform(uniform.name, uniform.type, (AssetHandle)NULL_UUID); break;
				default: break;
			}
		}
	}

	void MaterialUniformBlock::Serialize(YAML::Emitter& out)
	{
		out << YAML::Key << "Material Uniform Block" << YAML::BeginSeq;

		for (const auto& uniform : m_Uniforms)
		{
			const GLSLType& type = uniform.second.first;
			const UniformValue& value = uniform.second.second;



			out << YAML::BeginMap;
			out << YAML::Key << "Name" << YAML::Value << uniform.first;
			out << YAML::Key << "Type" << YAML::Value << Utils::GLSLTypeToString(type);

			switch (type)
			{
				// BOOL
				case GLSLType::Bool:
				{
					out << YAML::Key << "Value" << YAML::Value << std::get<bool>(value);

					break;
				}
				case GLSLType::BVec2:
				{
					glm::bvec2 temp = std::get<glm::bvec2>(value);

					out << YAML::Key << "Value" << YAML::Value << YAML::Flow
						<< YAML::BeginSeq
						<< temp[0]
						<< temp[1]
						<< YAML::EndSeq;


					break;
				}
				case GLSLType::BVec3:
				{
					glm::bvec3 temp = std::get<glm::bvec3>(value);

					out << YAML::Key << "Value" << YAML::Value << YAML::Flow
						<< YAML::BeginSeq
						<< temp[0]
						<< temp[1]
						<< temp[2]
						<< YAML::EndSeq;


					break;
				}
				case GLSLType::BVec4:
				{
					glm::bvec4 temp = std::get<glm::bvec4>(value);

					out << YAML::Key << "Value" << YAML::Value << YAML::Flow
						<< YAML::BeginSeq
						<< temp[0]
						<< temp[1]
						<< temp[2]
						<< temp[3]
						<< YAML::EndSeq;

					break;
				}

				// Int
				case GLSLType::Int:
				{
					out << YAML::Key << "Value" << YAML::Value << std::get<int>(value);

					break;
				}
				case GLSLType::IVec2:
				{
					glm::ivec2 temp = std::get<glm::ivec2>(value);

					out << YAML::Key << "Value" << YAML::Value << YAML::Flow
						<< YAML::BeginSeq
						<< temp[0]
						<< temp[1]
						<< YAML::EndSeq;

					break;
				}
				case GLSLType::IVec3:
				{
					glm::ivec3 temp = std::get<glm::ivec3>(value);

					out << YAML::Key << "Value" << YAML::Value << YAML::Flow
						<< YAML::BeginSeq
						<< temp[0]
						<< temp[1]
						<< temp[2]
						<< YAML::EndSeq;

					break;
				}
				case GLSLType::IVec4:
				{
					glm::ivec4 temp = std::get<glm::ivec4>(value);

					out << YAML::Key << "Value" << YAML::Value << YAML::Flow
						<< YAML::BeginSeq
						<< temp[0]
						<< temp[1]
						<< temp[2]
						<< temp[3]
						<< YAML::EndSeq;

					break;
				}

				// Unsigned Int
				case GLSLType::Uint:
				{
					out << YAML::Key << "Value" << YAML::Value << std::get<unsigned int>(value);

					break;
				}
				case GLSLType::UVec2:
				{
					glm::uvec2 temp = std::get<glm::uvec2>(value);

					out << YAML::Key << "Value" << YAML::Value << YAML::Flow
						<< YAML::BeginSeq
						<< temp[0]
						<< temp[1]
						<< YAML::EndSeq;

					break;
				}
				case GLSLType::UVec3:
				{
					glm::uvec3 temp = std::get<glm::uvec3>(value);

					out << YAML::Key << "Value" << YAML::Value << YAML::Flow
						<< YAML::BeginSeq
						<< temp[0]
						<< temp[1]
						<< temp[2]
						<< YAML::EndSeq;

					break;
				}
				case GLSLType::UVec4:
				{
					glm::uvec4 temp = std::get<glm::uvec4>(value);

					out << YAML::Key << "Value" << YAML::Value << YAML::Flow
						<< YAML::BeginSeq
						<< temp[0]
						<< temp[1]
						<< temp[2]
						<< temp[3]
						<< YAML::EndSeq;

					break;
				}

				// Float
				case GLSLType::Float:
				{
					out << YAML::Key << "Value" << YAML::Value << std::get<float>(value);

					break;
				}
				case GLSLType::Vec2:
				{
					glm::vec2 temp = std::get<glm::vec2>(value);

					out << YAML::Key << "Value" << YAML::Value << YAML::Flow
						<< YAML::BeginSeq
						<< temp[0]
						<< temp[1]
						<< YAML::EndSeq;

					break;
				}
				case GLSLType::Vec3:
				{
					glm::vec3 temp = std::get<glm::vec3>(value);

					out << YAML::Key << "Value" << YAML::Value << YAML::Flow
						<< YAML::BeginSeq
						<< temp[0]
						<< temp[1]
						<< temp[2]
						<< YAML::EndSeq;

					break;
				}
				case GLSLType::Vec4:
				{
					glm::vec4 temp = std::get<glm::vec4>(value);

					out << YAML::Key << "Value" << YAML::Value << YAML::Flow
						<< YAML::BeginSeq
						<< temp[0]
						<< temp[1]
						<< temp[2]
						<< temp[3]
						<< YAML::EndSeq;

					break;
				}

				// Double
				case GLSLType::Double:
				{
					out << YAML::Key << "Value" << YAML::Value << std::get<double>(value);

					break;
				}
				case GLSLType::DVec2:
				{
					glm::dvec2 temp = std::get<glm::dvec2>(value);

					out << YAML::Key << "Value" << YAML::Value << YAML::Flow
						<< YAML::BeginSeq
						<< temp[0]
						<< temp[1]
						<< YAML::EndSeq;

					break;
				}
				case GLSLType::DVec3:
				{
					glm::dvec3 temp = std::get<glm::dvec3>(value);

					out << YAML::Key << "Value" << YAML::Value << YAML::Flow
						<< YAML::BeginSeq
						<< temp[0]
						<< temp[1]
						<< temp[2]
						<< YAML::EndSeq;

					break;
				}
				case GLSLType::DVec4:
				{
					glm::dvec4 temp = std::get<glm::dvec4>(value);

					out << YAML::Key << "Value" << YAML::Value << YAML::Flow
						<< YAML::BeginSeq
						<< temp[0]
						<< temp[1]
						<< temp[2]
						<< temp[3]
						<< YAML::EndSeq;

					break;
				}

				// Matrix
				case GLSLType::Mat2:
				{
					glm::mat2 temp = std::get<glm::mat2>(value);
					out << YAML::Key << "Value" << YAML::Value << YAML::Flow;

					out << YAML::BeginSeq;
					for (int i = 0; i < 2; i++)
					{
						out << temp[i][0]
							<< temp[i][1];
					}
					out << YAML::EndSeq;

					break;
				}
				case GLSLType::Mat3:
				{
					glm::mat3 temp = std::get<glm::mat3>(value);
					out << YAML::Key << "Value" << YAML::Value << YAML::Flow;

					out << YAML::BeginSeq;
					for (int i = 0; i < 3; i++)
					{
						out << temp[i][0]
							<< temp[i][1]
							<< temp[i][2];
					}
					out << YAML::EndSeq;

					break;
				}
				case GLSLType::Mat4:
				{
					glm::mat4 temp = std::get<glm::mat4>(value);
					out << YAML::Key << "Value" << YAML::Value << YAML::Flow;
					
					out	<< YAML::BeginSeq;
					for (int i = 0; i < 4; i++)
					{
						out << temp[i][0]
							<< temp[i][1]
							<< temp[i][2]
							<< temp[i][3];
					}
					out << YAML::EndSeq;

					break;
				}

				// Texture Units
				case GLSLType::Sampler2D:
				case GLSLType::Sampler2DShadow:
				{
					out << YAML::Key << "Value" << YAML::Value << std::get<AssetHandle>(value);
					break;
				}

				case GLSLType::Sampler1D:
				case GLSLType::Sampler1DShadow:
				case GLSLType::Sampler1DArray:
				case GLSLType::Sampler1DArrayShadow:
				case GLSLType::Sampler2DArray:
				case GLSLType::Sampler2DArrayShadow:
				case GLSLType::Sampler3D:
				case GLSLType::SamplerCubeArray:
				case GLSLType::SamplerCube:
				case GLSLType::SamplerCubeShadow:
				case GLSLType::SamplerCubeArrayShadow:
				default:
				{
					out << YAML::Key << "Value" << YAML::Value << "Not Supported.";
					break;
				}
			}
			out << YAML::EndMap;
		}

		out << YAML::EndSeq;

	}

	void MaterialUniformBlock::Deserialize(YAML::Node data)
	{
		if (!data.IsSequence())
			return;

		for (const auto& node : data)
		{
			if (!node.IsMap())
				continue;

			std::string name = node["Name"].as<std::string>();
			GLSLType type = Utils::StringToGLSLType(node["Type"].as<std::string>());
			UniformValue value;

			if (m_Uniforms.count(name) == 0 || m_Uniforms.at(name).first != type)
				continue;

			auto valueNode = node["Value"];
			if (!valueNode)
				continue;

			switch (type)
			{
				case GLSLType::Bool:
				{
					value = valueNode.as<bool>();
					break;
				}
				case GLSLType::BVec2:
				{
					glm::bvec2 temp{};
					if (valueNode.IsSequence())
						for (int i = 0; i < valueNode.size(); i++)
							temp[i] = valueNode[i].as<bool>();
					value = temp;
					break;
				}
				case GLSLType::BVec3:
				{
					glm::bvec3 temp{};
					if (valueNode.IsSequence())
						for (int i = 0; i < valueNode.size(); i++)
							temp[i] = valueNode[i].as<bool>();
					value = temp;
					break;
				}
				case GLSLType::BVec4:
				{
					glm::bvec4 temp{};
					if (valueNode.IsSequence())
						for (int i = 0; i < valueNode.size(); i++)
							temp[i] = valueNode[i].as<bool>();
					value = temp;
					break;
				}
				case GLSLType::Int:
				{
					value = valueNode.as<int>();
					break;
				}
				case GLSLType::IVec2:
				{
					glm::ivec2 temp{};
					if (valueNode.IsSequence())
						for (int i = 0; i < valueNode.size(); i++)
							temp[i] = valueNode[i].as<int>();
					value = temp;
					break;
				}
				case GLSLType::IVec3:
				{
					glm::ivec3 temp{};
					if (valueNode.IsSequence())
						for (int i = 0; i < valueNode.size(); i++)
							temp[i] = valueNode[i].as<int>();
					value = temp;
					break;
				}
				case GLSLType::IVec4:
				{
					glm::ivec4 temp{};
					if (valueNode.IsSequence())
						for (int i = 0; i < valueNode.size(); i++)
							temp[i] = valueNode[i].as<int>();
					value = temp;
					break;
				}
				case GLSLType::Uint:
				{
					value = valueNode.as<unsigned int>();
					break;
				}
				case GLSLType::UVec2:
				{
					glm::uvec2 temp{};
					if (valueNode.IsSequence())
						for (int i = 0; i < valueNode.size(); i++)
							temp[i] = valueNode[i].as<unsigned int>();
					value = temp;
					break;
				}
				case GLSLType::UVec3:
				{
					glm::uvec3 temp{};
					if (valueNode.IsSequence())
						for (int i = 0; i < valueNode.size(); i++)
							temp[i] = valueNode[i].as<unsigned int>();
					value = temp;
					break;
				}
				case GLSLType::UVec4:
				{
					glm::uvec4 temp{};
					if (valueNode.IsSequence())
						for (int i = 0; i < valueNode.size(); i++)
							temp[i] = valueNode[i].as<unsigned int>();
					value = temp;
					break;
				}
				case GLSLType::Float:
				{
					value = valueNode.as<float>();
					break;
				}
				case GLSLType::Vec2:
				{
					glm::vec2 temp{};
					if (valueNode.IsSequence())
						for (int i = 0; i < valueNode.size(); i++)
							temp[i] = valueNode[i].as<float>();
					value = temp;
					break;
				}
				case GLSLType::Vec3:
				{
					glm::vec3 temp{};
					if (valueNode.IsSequence())
						for (int i = 0; i < valueNode.size(); i++)
							temp[i] = valueNode[i].as<float>();
					value = temp;
					break;
				}
				case GLSLType::Vec4:
				{
					glm::vec4 temp{};
					if (valueNode.IsSequence())
						for (int i = 0; i < valueNode.size(); i++)
							temp[i] = valueNode[i].as<float>();
					value = temp;
					break;
				}
				case GLSLType::Double:
				{
					value = valueNode.as<double>();
					break;
				}
				case GLSLType::DVec2:
				{
					glm::dvec2 temp{};
					if (valueNode.IsSequence())
						for (int i = 0; i < valueNode.size(); i++)
							temp[i] = valueNode[i].as<double>();
					value = temp;
					break;
				}
				case GLSLType::DVec3:
				{
					glm::dvec3 temp{};
					if (valueNode.IsSequence())
						for (int i = 0; i < valueNode.size(); i++)
							temp[i] = valueNode[i].as<double>();
					value = temp;
					break;
				}
				case GLSLType::DVec4:
				{
					glm::dvec4 temp{};
					if (valueNode.IsSequence())
						for (int i = 0; i < valueNode.size(); i++)
							temp[i] = valueNode[i].as<double>();
					value = temp;
					break;
				}
				case GLSLType::Mat2:
				{
					glm::mat2 temp{};

					if (valueNode.IsSequence())
						for (int i = 0; i < 2; i++)
							for (int j = 0; j < 2; j++)
								temp[i][j] = valueNode[(i * 2) + j].as<float>();

					value = temp;

					break;
				}
				case GLSLType::Mat3:
				{
					glm::mat3 temp{};

					if (valueNode.IsSequence())
						for (int i = 0; i < 3; i++)
							for (int j = 0; j < 3; j++)
								temp[i][j] = valueNode[(i * 3) + j].as<float>();

					value = temp;
				}
				case GLSLType::Mat4:
				{
					glm::mat4 temp{};

					if (valueNode.IsSequence())
						for (int i = 0; i < 4; i++)
							for (int j = 0; j < 4; j++)
								temp[i][j] = valueNode[(i * 4) + j].as<float>();

					value = temp;

					break;
				}
				case GLSLType::Sampler2D:
				case GLSLType::Sampler2DShadow:
					value = (AssetHandle)valueNode.as<uint32_t>();
					break;
				default:
					continue;
			}

			m_Uniforms[name] = { type, value };
		}
	}

}