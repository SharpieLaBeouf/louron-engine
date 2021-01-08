#pragma once

#include "Shader.h"
#include "Texture.h"

#include <string>
#include <unordered_map>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

enum TextureMapType {
	L20_TOTAL_ELEMENTS			= 3,
	L20_TEXTURE_DIFFUSE_MAP		= 0,
	L20_TEXTURE_SPECULAR_MAP	= 1,
	L20_TEXTURE_NORMAL_MAP		= 2
};

class Material {

public:

	GLboolean Bind();
	void UnBind();

	void setUniforms();

	Shader* getShader();

	Material() = delete;
	Material(Texture* texture);
	Material(Shader* shader, Texture* texture);
	Material(Shader* shader, std::unordered_map<GLint, Texture*>& textures);

	glm::vec4* getAmbient ();
	glm::vec4* getDiffuse ();
	glm::vec4* getSpecular();

	void setAmbient(const glm::vec4& val);
	void setDiffuse(const glm::vec4& val);
	void setSpecular(const glm::vec4& val);

	/// <param name="type">Refer using TextureMapType enum.</param>
	void AddTextureMap(GLint type, Texture* val);
	/// <param name="type">Refer using TextureMapType enum.</param>
	Texture* GetTextureMap(GLint type);

private:

	GLfloat m_Shine = 32.0f;
	glm::vec4 m_Ambient = glm::vec4(0.25f);
	glm::vec4 m_Diffuse = glm::vec4(1.0f);
	glm::vec4 m_Specular = glm::vec4(0.5f);

	Shader* m_Shader;

	std::unordered_map<GLint, Texture*> m_Textures;

	std::string m_TextureUniformNames[3] = { 
		"diffuseMap", 
		"specularMap", 
		"normalMap"
	};
};