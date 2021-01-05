#pragma once

#include "Shader.h"
#include "Texture.h"

#include <vector>
#include <string>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

class Material {

public:

	Material() {
		float m_Shine = 32.0f;

		m_Ambient = glm::vec3(1.0f);
		m_Diffuse = glm::vec3(1.0f);
		m_Specular = glm::vec3(1.0f);
	}

	float m_Shine;

	glm::vec3 m_Ambient;
	glm::vec3 m_Diffuse;
	glm::vec3 m_Specular;

	Texture m_DiffuseMap;
	Texture m_SpecularMap;
	Texture m_NormalMap;
	Texture m_HeightMap;
};