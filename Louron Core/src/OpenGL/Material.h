#pragma once

// Louron Core Headers
#include "Shader.h"
#include "Texture.h"
#include "../Asset/Asset.h"

// C++ Standard Library Headers
#include <string>
#include <array>
#include <variant>

// External Vendor Library Headers
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

namespace YAML {

	class Emitter;
	class Node;
}

namespace Louron {

	enum TextureMapType {
		L20_TOTAL_ELEMENTS = 3,
		L20_TEXTURE_DIFFUSE_MAP = 0,
		L20_TEXTURE_SPECULAR_MAP = 1,
		L20_TEXTURE_NORMAL_MAP = 2
	};

	enum RenderType {
		L_MATERIAL_OPAQUE = 0,
		L_MATERIAL_TRANSPARENT = 1,
		L_MATERIAL_INVALID = 2
	};

	static std::string RenderTypeToString(RenderType type) {

		switch (type) {

			case RenderType::L_MATERIAL_OPAQUE: return "Opaque";
			case RenderType::L_MATERIAL_TRANSPARENT: return "Transparent";

		}

		return "Invalid";
	}

	static RenderType StringToRenderType(const std::string& type_string) {

		if (type_string == "Opaque")
			return RenderType::L_MATERIAL_OPAQUE;

		if (type_string == "Transparent") 
			return RenderType::L_MATERIAL_TRANSPARENT;

		return RenderType::L_MATERIAL_INVALID;

	}

	class CameraBase;

	using UniformValue = std::variant<

		bool,
		glm::bvec2, glm::bvec3, glm::bvec4,

		int,
		glm::ivec2, glm::ivec3, glm::ivec4,

		unsigned int,
		glm::uvec2, glm::uvec3, glm::uvec4,

		float,
		glm::vec2, glm::vec3, glm::vec4,

		double,
		glm::dvec2, glm::dvec3, glm::dvec4,

		glm::mat2, glm::mat3, glm::mat4,

		AssetHandle
	>;

	using UniformBlock = std::unordered_map<std::string, std::pair<GLSLType, UniformValue>>;

	class MaterialUniformBlock {

	public:

		MaterialUniformBlock() = default;
		MaterialUniformBlock(const MaterialUniformBlock& other);
		MaterialUniformBlock(MaterialUniformBlock&& other);

		void SetUniform(const std::string& name, GLSLType type, UniformValue value) {
			m_Uniforms[name] = { type, value };
		}

		const UniformBlock& GetUniforms() const {
			return m_Uniforms;
		}

		void SetUniforms(const UniformBlock& uniforms) {
			m_Uniforms = uniforms;
		}

		void Clear() {
			m_Uniforms.clear();
		}

		void InitialiseFromShader(std::shared_ptr<Shader> shader);

		void Serialize(YAML::Emitter& out);
		void Deserialize(YAML::Node data);

	private:
		UniformBlock m_Uniforms;

		friend class Material;
		friend class PBRMaterial;
	};


	class Material : public Asset {

	public:

		Material();
		Material(const std::string& name, std::shared_ptr<Shader> shader);
		Material(AssetHandle shader);
		Material(std::shared_ptr<Texture> texture);
		Material(AssetHandle shader, std::shared_ptr<Texture> texture);
		Material(AssetHandle shader, std::unordered_map<GLint, std::shared_ptr<Texture>>& textures);

		virtual AssetType GetType() const override { return AssetType::Material_Standard; }

	public:

		// Bind and Unbing
		virtual GLboolean Bind();
		virtual void UnBind();

		// Update Material Shader Uniforms
		virtual void UpdateUniforms(const glm::vec3& camera_position, const glm::mat4& projection_matrix, const glm::mat4& view_matrix, std::shared_ptr<MaterialUniformBlock> custom_uniform_block = nullptr);

		std::shared_ptr<MaterialUniformBlock> GetUniformBlock() { return m_UniformBlock; }

		// Getters and Setters

		void SetTexture(std::shared_ptr<Texture> texture, TextureMapType textureType);

		virtual std::shared_ptr<Shader> GetShader();
		virtual AssetHandle GetShaderHandle();
		void SetShader(AssetHandle shader);

		float GetShine() const;
		void SetShine(float shine);

		glm::vec4* GetDiffuse(); 
		void SetDiffuse(const glm::vec4& val);

		glm::vec4* GetSpecular();
		void SetSpecular(const glm::vec4& val);

		void AddTextureMap(GLint type, std::shared_ptr<Texture> val);
		std::shared_ptr<Texture> GetTextureMap(GLint type);

		std::string GetName() const { return m_Name; }
		void SetName(const std::string& name) { m_Name = name; }

		RenderType GetRenderType() const { return m_RenderType; }
		void SetRenderType(RenderType type) { m_RenderType = type; }

	private:

		std::string m_Name;

		GLfloat m_Shine = 32.0f;
		glm::vec4 m_Diffuse = glm::vec4(1.0f);
		glm::vec4 m_Specular = glm::vec4(0.5f);

		std::unordered_map<GLint, std::shared_ptr<Texture>> m_Textures;

		std::string m_TextureUniformNames[3] = {
			"diffuseMap",
			"specularMap",
			"normalMap"
		};

	protected:

		AssetHandle m_ShaderAssetHandle;
		RenderType m_RenderType = RenderType::L_MATERIAL_OPAQUE;

		std::shared_ptr<MaterialUniformBlock> m_UniformBlock = nullptr;

	};

	class PBRMaterial : public Material {

	public:

		PBRMaterial();
		PBRMaterial(AssetHandle shader_handle);

		virtual AssetType GetType() const override { return AssetType::Material_Standard; }

	private:

		std::string m_MaterialName = "New PBR Material";
		
		float m_Roughness = 0.5f;
		float m_MetallicScale = 0.0f;
		glm::vec4 m_AlbedoTint = glm::vec4(1.0f);

		AssetHandle m_AlbedoTexture = NULL_UUID;
		AssetHandle m_MetallicTexture = NULL_UUID;
		AssetHandle m_NormalTexture = NULL_UUID;

	public:

		// Bind and Unbing
		GLboolean Bind() override;
		void UnBind() override;

		// Update Material Shader Uniforms
		void UpdateUniforms(const glm::vec3& camera_position, const glm::mat4& projection_matrix, const glm::mat4& view_matrix, std::shared_ptr<MaterialUniformBlock> custom_uniform_block = nullptr) override;

		bool IsAlbedoTextureSet() const;
		bool IsMetallicTextureSet() const;
		bool IsNormalTextureSet() const;

		void SetAlbedoTexture(AssetHandle texture);
		void SetMetallicTexture(AssetHandle texture);
		void SetNormalTexture(AssetHandle texture);

		void SetRoughness(float roughness);
		void SetMetallic(float metallic);
		void SetAlbedoTintColour(const glm::vec4& albedo_colour);

		void SetShader(AssetHandle shader_handle);
		void SetName(const std::string& name);

		AssetHandle GetAlbedoTextureAssetHandle() const;
		AssetHandle GetMetallicTextureAssetHandle() const;
		AssetHandle GetNormalTextureAssetHandle() const;

		float GetRoughness() const;
		float GetMetallic() const;
		const glm::vec4& GetAlbedoTintColour() const;

		std::shared_ptr<Shader> GetShader() override;
		AssetHandle GetShaderHandle() override;
		const std::string& GetName() const;

		void Serialize(YAML::Emitter& out);
		bool Deserialize(const std::filesystem::path& path);

	};

}