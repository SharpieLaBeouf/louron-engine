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

	enum RenderType : uint8_t {
		L_MATERIAL_OPAQUE = 0,
		L_MATERIAL_TRANSPARENT,
		L_MATERIAL_TRANSPARENT_WRITE_DEPTH,
		L_MATERIAL_INVALID
	};

	static std::string RenderTypeToString(RenderType type) {

		switch (type)
		{
			case RenderType::L_MATERIAL_OPAQUE: return "Opaque";
			case RenderType::L_MATERIAL_TRANSPARENT: return "Transparent";
			case RenderType::L_MATERIAL_TRANSPARENT_WRITE_DEPTH: return "TransparentWriteDepth";
		}

		return "Invalid";
	}

	static RenderType StringToRenderType(const std::string& type_string) {

		if (type_string == "Opaque")
			return RenderType::L_MATERIAL_OPAQUE;

		if (type_string == "Transparent") 
			return RenderType::L_MATERIAL_TRANSPARENT;

		if (type_string == "TransparentWriteDepth")
			return RenderType::L_MATERIAL_TRANSPARENT_WRITE_DEPTH;

		return RenderType::L_MATERIAL_INVALID;

	}

	class CameraBase;

	// TODO: Change to void* opposed to variant? We can then cast to 
	// the type based on the GLSLType stored alongside the Uniform?
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

		MaterialUniformBlock() { m_UniformBlockID = UUID(); }
		MaterialUniformBlock(const MaterialUniformBlock& other);
		MaterialUniformBlock(MaterialUniformBlock&& other) noexcept;

		MaterialUniformBlock& operator=(const MaterialUniformBlock& other);
		MaterialUniformBlock& operator=(MaterialUniformBlock&& other) noexcept;

		void SetUniform(const std::string& name, GLSLType type, UniformValue value) {
			m_Uniforms[name] = { type, value };
		}

		const UniformBlock& GetUniforms() const {
			return m_Uniforms;
		}

		const UniformBlock& GetMaterialOverideUniforms() const {
			return m_MaterialOveriddes;
		}

		void SetUniforms(const UniformBlock& uniforms) {
			m_Uniforms = uniforms;
		}

		void OverrideAlbedoMap(const AssetHandle& new_albedo);
		void OverrideMetallicMap(const AssetHandle& new_metallic);
		void OverrideNormalMap(const AssetHandle& new_normal);

		void OverrideAlbedoTint(const glm::vec4& new_albedo_tint);
		void OverrideMetallic(float new_metallic_scale);
		void OverrideRoughness(float new_roughness);

		void Clear() {
			m_Uniforms.clear();
			m_MaterialOveriddes.clear();
		}

		void InitialiseFromShader(std::shared_ptr<Shader> shader);

		void Serialize(YAML::Emitter& out);
		void Deserialize(YAML::Node data);

		bool operator==(const MaterialUniformBlock& other) const { 
			return m_UniformBlockID == other.m_UniformBlockID; 
		}

		bool operator<(const MaterialUniformBlock& other) const { return m_UniformBlockID < other.m_UniformBlockID; }

		void GenerateNewBlockID() {
			m_UniformBlockID = UUID();
		}

	private:
		UniformBlock m_Uniforms;
		UniformBlock m_MaterialOveriddes;
		UUID m_UniformBlockID = NULL_UUID;

		friend class Material;
	};

	class Material : public Asset {

	public:

		Material();
		Material(AssetHandle shader_handle);
		Material(const Material& other);
		Material(Material&& other) noexcept;

		Material& operator=(const Material& other);
		Material& operator=(Material&& other) noexcept;

		virtual AssetType GetType() const override { return AssetType::Material_Standard; }

	private:

		std::string m_MaterialName = "New Material";

		std::shared_ptr<MaterialUniformBlock> m_UniformBlock = nullptr;

		glm::vec4 m_AlbedoTint = glm::vec4(1.0f);

		AssetHandle m_AlbedoTexture = NULL_UUID;
		AssetHandle m_MetallicTexture = NULL_UUID;
		AssetHandle m_NormalTexture = NULL_UUID;

		float m_Roughness = 0.5f;
		float m_MetallicScale = 0.0f;

	protected: // Alignment purposes

		AssetHandle m_ShaderAssetHandle;

	private: // Alignment purposes

		RenderType m_RenderType = RenderType::L_MATERIAL_OPAQUE;

	public:

		// Bind and Unbing
		virtual bool Bind() const;
		virtual void UnBind() const;

		// Update Material Shader Uniforms
		virtual void UpdateUniforms(std::shared_ptr<MaterialUniformBlock> custom_uniform_block = nullptr);

		void SetUniformBlock(std::shared_ptr<MaterialUniformBlock> uniform_block) { m_UniformBlock = uniform_block; }
		std::shared_ptr<MaterialUniformBlock> GetUniformBlock() { return m_UniformBlock; }

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

		RenderType GetRenderType() const { return m_RenderType; }
		void SetRenderType(RenderType type) { m_RenderType = type; }

		void SetTransparencyWriteDepth(bool write_depth);
		bool IsTransparencyWriteDepth() const;

		float GetRoughness() const;
		float GetMetallic() const;
		const glm::vec4& GetAlbedoTintColour() const;

		std::shared_ptr<Shader> GetShader() const;
		AssetHandle GetShaderHandle();
		const std::string& GetName() const;

		virtual void Serialize(YAML::Emitter& out) const;
		virtual bool Deserialize(const std::filesystem::path& path);

	};

}