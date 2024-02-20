#pragma once

#include "Shader.h"
#include "Texture.h"

#include <string>
#include <array>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

namespace Louron {

	enum TextureMapType {
		L20_TOTAL_ELEMENTS = 3,
		L20_TEXTURE_DIFFUSE_MAP = 0,
		L20_TEXTURE_SPECULAR_MAP = 1,
		L20_TEXTURE_NORMAL_MAP = 2
	};

	enum RenderType {
		L_MATERIAL_OPAQUE = 0,
		L_MATERIAL_TRANSPARENT = 1
	};

	class Camera;

	class Material {

	public:

		Material();
		Material(const std::string& name, std::shared_ptr<Shader> shader);
		Material(std::shared_ptr<Shader> shader);
		Material(std::shared_ptr<Texture> texture);
		Material(std::shared_ptr<Shader> shader, std::shared_ptr<Texture> texture);
		Material(std::shared_ptr<Shader> shader, std::unordered_map<GLint, std::shared_ptr<Texture>>& textures);

	public:

		// Bind and Unbing
		GLboolean Bind();
		void UnBind();

		// Update Material Shader Uniforms
		void UpdateUniforms(const Camera& Camera);

		// Getters and Setters

		void SetTexture(std::shared_ptr<Texture> texture, TextureMapType textureType);

		std::shared_ptr<Shader>& GetShader();
		void SetShader(std::shared_ptr<Shader>& shader);

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

	private:

		RenderType m_RenderType = RenderType::L_MATERIAL_OPAQUE;

		std::string m_Name;

		GLfloat m_Shine = 32.0f;
		glm::vec4 m_Diffuse = glm::vec4(1.0f);
		glm::vec4 m_Specular = glm::vec4(0.5f);

		std::shared_ptr<Shader> m_Shader;
		std::unordered_map<GLint, std::shared_ptr<Texture>> m_Textures;

		std::string m_TextureUniformNames[3] = {
			"diffuseMap",
			"specularMap",
			"normalMap"
		};
	};

	class BPMaterial : public Material {

	public:

		// Constructors and Logic

		BPMaterial() = default;
		~BPMaterial() = default;

	public:

		// Getters and Setters

		void SetMaterialName(const std::string& name) { m_MaterialName = name; }
		const std::string& GetMaterialName() const { return m_MaterialName; }

		void SetRenderType(const RenderType& renderType) { m_RenderType = renderType; }
		const RenderType& GetRenderType() const { return m_RenderType; }

		void SetShine(const GLfloat& shine) { m_Shine = shine; }
		const GLfloat& GetShine() const { return m_Shine; }

		void SetDiffuse(const glm::vec4& diffuse) { m_Diffuse = diffuse; }
		const glm::vec4& GetDiffuse() const { return m_Diffuse; }

		void SetSpecular(const glm::vec4& specular) { m_Specular = specular; }
		const glm::vec4& GetSpecular() const { return m_Specular; }

		void SetShader(std::shared_ptr<Shader> shader) { m_MaterialShader = shader; }
		std::shared_ptr<Shader> GetShader(std::shared_ptr<Shader> shader) { return m_MaterialShader; }

	private:

		std::string m_MaterialName = "New Blinn Phong Material";
		RenderType m_RenderType = RenderType::L_MATERIAL_OPAQUE;

		GLfloat m_Shine = 32.0f;
		glm::vec4 m_Diffuse = glm::vec4(1.0f);
		glm::vec4 m_Specular = glm::vec4(0.5f);

		GLuint m_MaterialIndex;
		std::shared_ptr<Shader> m_MaterialShader;
		std::array<std::shared_ptr<Texture>, L20_TOTAL_ELEMENTS> m_MaterialTextures;


	};

	struct PBRMaterial : public Material {

	public:

		PBRMaterial() = default;
		~PBRMaterial() = default;

	};

}