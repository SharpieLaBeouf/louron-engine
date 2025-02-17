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

#pragma region PBR Materials

	Material::Material()
	{
		SetShader(AssetManager::GetInbuiltShader("FP_Material_PBR_Shader")->Handle);
	}

	Material::Material(AssetHandle shader_handle)
	{
		SetShader(shader_handle);
	}

	Material::Material(const Material& other)
	{
		Handle = other.Handle;

		m_MaterialName = other.m_MaterialName;
		m_UniformBlock = other.m_UniformBlock;
		m_AlbedoTint = other.m_AlbedoTint;
		m_AlbedoTexture = other.m_AlbedoTexture;
		m_MetallicTexture = other.m_MetallicTexture;
		m_NormalTexture = other.m_NormalTexture;
		m_Roughness = other.m_Roughness;
		m_MetallicScale = other.m_MetallicScale;
		m_ShaderAssetHandle = other.m_ShaderAssetHandle;
		m_RenderType = other.m_RenderType;
	}

	Material::Material(Material&& other) noexcept
	{
		Handle = other.Handle; other.Handle = NULL_UUID;

		m_MaterialName = other.m_MaterialName;				other.m_MaterialName = "New Material";
		m_UniformBlock = other.m_UniformBlock;				other.m_UniformBlock = nullptr;
		m_AlbedoTint = other.m_AlbedoTint;					other.m_AlbedoTint = glm::vec4(1.0f);
		m_AlbedoTexture = other.m_AlbedoTexture;			other.m_AlbedoTexture = NULL_UUID;
		m_MetallicTexture = other.m_MetallicTexture;		other.m_MetallicTexture = NULL_UUID;
		m_NormalTexture = other.m_NormalTexture;			other.m_NormalTexture = NULL_UUID;
		m_Roughness = other.m_Roughness;					other.m_Roughness = 0.5f;
		m_MetallicScale = other.m_MetallicScale;			other.m_MetallicScale = 0.0f;
		m_ShaderAssetHandle = other.m_ShaderAssetHandle;	other.m_ShaderAssetHandle = AssetManager::GetInbuiltShader("FP_Material_PBR_Shader")->Handle;
		m_RenderType = other.m_RenderType;					other.m_RenderType = RenderType::L_MATERIAL_OPAQUE;
	}

	Material& Material::operator=(const Material& other)
	{
		if (this == &other)
			return *this;

		Handle = other.Handle;

		m_MaterialName = other.m_MaterialName;
		m_UniformBlock = other.m_UniformBlock;
		m_AlbedoTint = other.m_AlbedoTint;
		m_AlbedoTexture = other.m_AlbedoTexture;
		m_MetallicTexture = other.m_MetallicTexture;
		m_NormalTexture = other.m_NormalTexture;
		m_Roughness = other.m_Roughness;
		m_MetallicScale = other.m_MetallicScale;
		m_ShaderAssetHandle = other.m_ShaderAssetHandle;
		m_RenderType = other.m_RenderType;

		return *this;
	}

	Material& Material::operator=(Material&& other) noexcept
	{
		if (this == &other)
			return *this;

		Handle = other.Handle; other.Handle = NULL_UUID;

		m_MaterialName = other.m_MaterialName;				other.m_MaterialName = "New Material";
		m_UniformBlock = other.m_UniformBlock;				other.m_UniformBlock = nullptr;
		m_AlbedoTint = other.m_AlbedoTint;					other.m_AlbedoTint = glm::vec4(1.0f);
		m_AlbedoTexture = other.m_AlbedoTexture;			other.m_AlbedoTexture = NULL_UUID;
		m_MetallicTexture = other.m_MetallicTexture;		other.m_MetallicTexture = NULL_UUID;
		m_NormalTexture = other.m_NormalTexture;			other.m_NormalTexture = NULL_UUID;
		m_Roughness = other.m_Roughness;					other.m_Roughness = 0.5f;
		m_MetallicScale = other.m_MetallicScale;			other.m_MetallicScale = 0.0f;
		m_ShaderAssetHandle = other.m_ShaderAssetHandle;	other.m_ShaderAssetHandle = AssetManager::GetInbuiltShader("FP_Material_PBR_Shader")->Handle;
		m_RenderType = other.m_RenderType;					other.m_RenderType = RenderType::L_MATERIAL_OPAQUE;

		return *this;
	}

	bool Material::Bind() const 
	{
		if(auto shader_ref = AssetManager::GetAsset<Shader>(m_ShaderAssetHandle); shader_ref) {
			shader_ref->Bind();
			return true;
		}
		L_CORE_ERROR("Shader Not Found for PBR Material: {0}", m_MaterialName);
		return false;
	}

	void Material::UnBind() const
	{
		for (int i = 0; i < 3; i++) {
			glActiveTexture(GL_TEXTURE0 + i);
			glBindTexture(GL_TEXTURE_2D, 0);
		}

		glActiveTexture(GL_TEXTURE0);
		glUseProgram(0);
	}

	static AssetHandle default_texture_handle = static_cast<uint32_t>(std::hash<std::string>{}(
		AssetUtils::AssetTypeToString(AssetType::Texture2D) + "InBuiltAsset" + "Default_White_Texture"
	));
	static AssetHandle default_normal_texture_handle = static_cast<uint32_t>(std::hash<std::string>{}(
		AssetUtils::AssetTypeToString(AssetType::Texture2D) + "InBuiltAsset" + "Default_Normal_Texture"
	));

	static void SetUniforms(const UniformBlock& uniform_block, const Shader& shader_ref, uint8_t texture_unit, uint8_t max_texture_units)
	{
		for (const auto& uniform : uniform_block)
		{
			std::string name = "u_MaterialUniforms." + uniform.first;
			const GLSLType& type = uniform.second.first;
			const UniformValue& value = uniform.second.second;

			switch (type)

			{
				case GLSLType::Bool:	shader_ref.SetBool(name.c_str(), std::get<bool>(value)); break;
				case GLSLType::BVec2:	shader_ref.SetBoolVec2(name.c_str(), std::get<glm::bvec2>(value)); break;
				case GLSLType::BVec3:	shader_ref.SetBoolVec3(name.c_str(), std::get<glm::bvec3>(value)); break;
				case GLSLType::BVec4:	shader_ref.SetBoolVec4(name.c_str(), std::get<glm::bvec4>(value)); break;

				case GLSLType::Int:		shader_ref.SetInt(name.c_str(), std::get<int>(value)); break;
				case GLSLType::IVec2:	shader_ref.SetIntVec2(name.c_str(), std::get<glm::ivec2>(value)); break;
				case GLSLType::IVec3:	shader_ref.SetIntVec3(name.c_str(), std::get<glm::ivec3>(value)); break;
				case GLSLType::IVec4:	shader_ref.SetIntVec4(name.c_str(), std::get<glm::ivec4>(value)); break;

				case GLSLType::Uint:	shader_ref.SetUInt(name.c_str(), std::get<GLuint>(value)); break;
				case GLSLType::UVec2:	shader_ref.SetUIntVec2(name.c_str(), std::get<glm::uvec2>(value)); break;
				case GLSLType::UVec3:	shader_ref.SetUIntVec3(name.c_str(), std::get<glm::uvec3>(value)); break;
				case GLSLType::UVec4:	shader_ref.SetUIntVec4(name.c_str(), std::get<glm::uvec4>(value)); break;

				case GLSLType::Float:	shader_ref.SetFloat(name.c_str(), std::get<float>(value)); break;
				case GLSLType::Vec2:	shader_ref.SetFloatVec2(name.c_str(), std::get<glm::vec2>(value)); break;
				case GLSLType::Vec3:	shader_ref.SetFloatVec3(name.c_str(), std::get<glm::vec3>(value)); break;
				case GLSLType::Vec4:	shader_ref.SetFloatVec4(name.c_str(), std::get<glm::vec4>(value)); break;

				case GLSLType::Double:	shader_ref.SetDouble(name.c_str(), std::get<double>(value)); break;
				case GLSLType::DVec2:	shader_ref.SetDoubleVec2(name.c_str(), std::get<glm::dvec2>(value)); break;
				case GLSLType::DVec3:	shader_ref.SetDoubleVec3(name.c_str(), std::get<glm::dvec3>(value)); break;
				case GLSLType::DVec4:	shader_ref.SetDoubleVec4(name.c_str(), std::get<glm::dvec4>(value)); break;

				case GLSLType::Mat2:	shader_ref.SetMat2(name.c_str(), std::get<glm::mat2>(value)); break;
				case GLSLType::Mat3:	shader_ref.SetMat3(name.c_str(), std::get<glm::mat3>(value)); break;
				case GLSLType::Mat4:	shader_ref.SetMat4(name.c_str(), std::get<glm::mat4>(value)); break;

				case GLSLType::Sampler2D:
				case GLSLType::Sampler2DShadow:
				{
					AssetHandle texture_handle = std::get<AssetHandle>(value);

					if (auto texture_ref = AssetManager::GetAsset<Texture2D>(texture_handle); texture_ref && *texture_ref && texture_unit < max_texture_units) {
						glActiveTexture(GL_TEXTURE0 + texture_unit);
						shader_ref.SetInt(name.c_str(), texture_unit);
						texture_ref->Bind();
						texture_unit++;
					}
					else
					{
						glActiveTexture(GL_TEXTURE0 + texture_unit);
						shader_ref.SetInt(name.c_str(), texture_unit);
						AssetManager::GetAsset<Texture2D>(default_texture_handle)->Bind();
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

	void Material::UpdateUniforms(std::shared_ptr<MaterialUniformBlock> custom_uniform_block) 
	{
		if (auto shader_ref = AssetManager::GetAsset<Shader>(m_ShaderAssetHandle); shader_ref) 
		{
			const UniformBlock& material_override_uniforms = (custom_uniform_block) ? custom_uniform_block->GetMaterialOverideUniforms() : m_UniformBlock->GetMaterialOverideUniforms();

			shader_ref->SetFloatVec4("u_Material.albedoTint", custom_uniform_block && material_override_uniforms.contains("u_Material.albedoTint")  ?
				std::get<glm::vec4>(material_override_uniforms.at("u_Material.albedoTint").second) :
				m_AlbedoTint
			);
			shader_ref->SetFloat("u_Material.roughness", custom_uniform_block && material_override_uniforms.contains("u_Material.roughness") ?
				std::get<float>(material_override_uniforms.at("u_Material.roughness").second) :
				m_Roughness
			);
			shader_ref->SetFloat("u_Material.metallicScale", IsMetallicTextureSet() ? 1.0f : custom_uniform_block && material_override_uniforms.contains("u_Material.metallicScale") ?
				std::get<float>(material_override_uniforms.at("u_Material.metallicScale").second) :
				m_MetallicScale
			);

			GLint max_texture_units = 3;
			glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &max_texture_units);

			GLint texture_unit = 0;

			AssetHandle texture_handle = custom_uniform_block && material_override_uniforms.contains("u_Material.albedoTexture") ?
				std::get<AssetHandle>(material_override_uniforms.at("u_Material.albedoTexture").second) :
				m_AlbedoTexture;

			if (auto texture_ref = (texture_handle == NULL_UUID) ? AssetManager::GetAsset<Texture2D>(default_texture_handle) : AssetManager::GetAsset<Texture2D>(texture_handle); texture_ref && *texture_ref && texture_unit < max_texture_units) {
				glActiveTexture(GL_TEXTURE0 + texture_unit);
				shader_ref->SetInt("u_Material.albedoTexture", texture_unit);
				texture_ref->Bind();
				texture_unit++;
			}

			texture_handle = custom_uniform_block && material_override_uniforms.contains("u_Material.metallicTexture") ?
				std::get<AssetHandle>(material_override_uniforms.at("u_Material.metallicTexture").second) :
				m_MetallicTexture;

			if (auto texture_ref = (texture_handle == NULL_UUID) ? AssetManager::GetAsset<Texture2D>(default_texture_handle) : AssetManager::GetAsset<Texture2D>(texture_handle); texture_ref && *texture_ref && texture_unit < max_texture_units) {
				glActiveTexture(GL_TEXTURE0 + texture_unit);
				shader_ref->SetInt("u_Material.metallicTexture", texture_unit);
				texture_ref->Bind();
				texture_unit++;
			}

			texture_handle = custom_uniform_block && material_override_uniforms.contains("u_Material.normalTexture") ?
				std::get<AssetHandle>(material_override_uniforms.at("u_Material.normalTexture").second) :
				m_NormalTexture;

			if (auto texture_ref = (texture_handle == NULL_UUID) ? AssetManager::GetAsset<Texture2D>(default_normal_texture_handle) : AssetManager::GetAsset<Texture2D>(texture_handle); texture_ref && *texture_ref && texture_unit < max_texture_units) {
				glActiveTexture(GL_TEXTURE0 + texture_unit);
				shader_ref->SetInt("u_Material.normalTexture", texture_unit);
				texture_ref->Bind();
				texture_unit++;
			}

			// All other texture units are pre-dedicated to other things such as depth map and shadow maps. 
			texture_unit = 7;

			const UniformBlock& custom_uniforms = (custom_uniform_block) ? custom_uniform_block->GetUniforms() : m_UniformBlock->GetUniforms();
			Louron::SetUniforms(custom_uniforms, *shader_ref, texture_unit, max_texture_units);
		}
		else 
		{
			L_CORE_ERROR("Shader Not Found for PBR Material: {0}", GetName());
		}

	}

	bool Material::IsAlbedoTextureSet() const { return m_AlbedoTexture != NULL_UUID; }
	bool Material::IsMetallicTextureSet() const { return m_MetallicTexture != NULL_UUID; }
	bool Material::IsNormalTextureSet() const { return m_NormalTexture != NULL_UUID; }

	#pragma region Getters and Setters

	void Material::SetAlbedoTexture(AssetHandle texture) { m_AlbedoTexture = texture; }
	void Material::SetMetallicTexture(AssetHandle texture) { m_MetallicTexture = texture; }
	void Material::SetNormalTexture(AssetHandle texture) { m_NormalTexture = texture; }

	void Material::SetRoughness(float roughness) { m_Roughness = roughness; }
	void Material::SetMetallic(float metallic) { m_MetallicScale = metallic; }
	void Material::SetAlbedoTintColour(const glm::vec4& albedo_colour) { m_AlbedoTint = albedo_colour; }

	void Material::SetName(const std::string& name) { m_MaterialName = name; }
	void Material::SetShader(AssetHandle shader_handle) 
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

	AssetHandle Material::GetAlbedoTextureAssetHandle() const { return m_AlbedoTexture; }

	AssetHandle Material::GetMetallicTextureAssetHandle() const { return m_MetallicTexture; }

	AssetHandle Material::GetNormalTextureAssetHandle() const { return m_NormalTexture; }

	void Material::SetTransparencyWriteDepth(bool write_depth)
	{
		if (m_RenderType == L_MATERIAL_TRANSPARENT || m_RenderType == L_MATERIAL_TRANSPARENT_WRITE_DEPTH)
		{
			m_RenderType = (write_depth) ? L_MATERIAL_TRANSPARENT_WRITE_DEPTH : L_MATERIAL_TRANSPARENT;
		}
	}

	bool Material::IsTransparencyWriteDepth() const
	{
		return (m_RenderType == L_MATERIAL_TRANSPARENT_WRITE_DEPTH);
	}

	float Material::GetRoughness() const { return m_Roughness; }
	float Material::GetMetallic() const { return m_MetallicScale; }
	const glm::vec4& Material::GetAlbedoTintColour() const { return m_AlbedoTint; }

	std::shared_ptr<Shader> Material::GetShader() const {
		return AssetManager::GetAsset<Shader>(m_ShaderAssetHandle);
	}

	AssetHandle Material::GetShaderHandle()
	{
		return m_ShaderAssetHandle;
	}

	const std::string& Material::GetName() const { return m_MaterialName; }

	void Material::Serialize(YAML::Emitter& out) const
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

	bool Material::Deserialize(const std::filesystem::path& path)
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
			SetTransparencyWriteDepth(m_RenderType == L_MATERIAL_TRANSPARENT_WRITE_DEPTH);
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
		m_UniformBlockID = other.m_UniformBlockID;
	}

	MaterialUniformBlock::MaterialUniformBlock(MaterialUniformBlock&& other) noexcept
	{
		m_Uniforms = other.m_Uniforms; other.m_Uniforms = {};
		m_UniformBlockID = other.m_UniformBlockID; other.m_UniformBlockID = NULL_UUID;
	}

	MaterialUniformBlock& Louron::MaterialUniformBlock::operator=(const MaterialUniformBlock& other)
	{
		if (this == &other)
			return *this;

		m_Uniforms = other.m_Uniforms;
		m_UniformBlockID = other.m_UniformBlockID;

		return *this;
	}

	MaterialUniformBlock& Louron::MaterialUniformBlock::operator=(MaterialUniformBlock&& other) noexcept
	{
		if (this == &other)
			return *this;

		m_Uniforms = other.m_Uniforms; other.m_Uniforms = {};
		m_UniformBlockID = other.m_UniformBlockID; other.m_UniformBlockID = NULL_UUID;

		return *this;
	}

	void MaterialUniformBlock::OverrideAlbedoMap(const AssetHandle& new_albedo)
	{
		if (!AssetManager::IsAssetHandleValid(new_albedo))
			return;

		m_MaterialOveriddes["u_Material.albedoTexture"] = { GLSLType::Sampler2D, new_albedo };
	}

	void MaterialUniformBlock::OverrideMetallicMap(const AssetHandle& new_metallic)
	{
		if (!AssetManager::IsAssetHandleValid(new_metallic))
			return;

		m_MaterialOveriddes["u_Material.metallicTexture"] = { GLSLType::Sampler2D, new_metallic };
	}


	void MaterialUniformBlock::OverrideNormalMap(const AssetHandle& new_normal)
	{
		if (!AssetManager::IsAssetHandleValid(new_normal))
			return;

		m_MaterialOveriddes["u_Material.normalTexture"] = { GLSLType::Sampler2D, new_normal };
	}

	void Louron::MaterialUniformBlock::OverrideAlbedoTint(const glm::vec4& new_albedo_tint)
	{
		m_MaterialOveriddes["u_Material.albedoTint"] = { GLSLType::Vec4, new_albedo_tint };
	}

	void Louron::MaterialUniformBlock::OverrideMetallic(float new_metallic_scale)
	{
		m_MaterialOveriddes["u_Material.metallicScale"] = { GLSLType::Float, new_metallic_scale };
	}

	void Louron::MaterialUniformBlock::OverrideRoughness(float new_roughness)
	{
		m_MaterialOveriddes["u_Material.roughness"] = { GLSLType::Float, new_roughness };
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
