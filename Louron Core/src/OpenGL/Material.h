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

	struct CameraComponent;

	class Material {

	public:

		GLboolean Bind();
		void UnBind();

		void SetUniforms();
		void UpdateUniforms(const CameraComponent& Camera);

		void SetShader(std::shared_ptr<Shader>& shader);
		std::shared_ptr<Shader>& GetShader();

		Material();
		Material(std::shared_ptr<Shader>& shader);
		Material(Texture* texture);
		Material(std::shared_ptr<Shader>& shader, Texture* texture);
		Material(std::shared_ptr<Shader>& shader, std::unordered_map<GLint, Texture*>& textures);

		float GetShine() const;
		glm::vec4* GetDiffuse();
		glm::vec4* GetSpecular();

		void SetTexture(Texture* texture, TextureMapType textureType);

		void SetShine(float shine);
		void SetDiffuse(const glm::vec4& val);
		void SetSpecular(const glm::vec4& val);

		void AddTextureMap(GLint type, Texture* val);
		
		Texture* GetTextureMap(GLint type);

		void SetMaterialIndex(GLuint index);
		GLuint GetMaterialIndex() const;

	private:

		GLuint m_MaterialIndex = NULL;

		GLfloat m_Shine = 32.0f;
		glm::vec4 m_Diffuse = glm::vec4(1.0f);
		glm::vec4 m_Specular = glm::vec4(0.5f);

		std::shared_ptr<Shader> m_Shader;

		std::unordered_map<GLint, Texture*> m_Textures;

		std::string m_TextureUniformNames[3] = {
			"diffuseMap",
			"specularMap",
			"normalMap"
		};
	};

	struct BPMaterial : public Material {

	public:

		BPMaterial() = default;
		~BPMaterial() = default;

		GLfloat m_Shine = 32.0f;
		glm::vec4 m_Diffuse = glm::vec4(1.0f);
		glm::vec4 m_Specular = glm::vec4(0.5f);

		GLuint MaterialIndex;
		std::shared_ptr<Shader> MaterialShader;
		std::array<std::shared_ptr<Texture>, L20_TOTAL_ELEMENTS> MaterialTextures;

	public:

		void SetShader(std::shared_ptr<Shader> shader) { MaterialShader = shader; }

	};

	struct PBRMaterial : public Material {

	public:

		PBRMaterial() = default;
		~PBRMaterial() = default;

	};

}