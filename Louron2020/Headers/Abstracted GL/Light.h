#pragma once

#include "Shader.h"
#include "Texture.h"

#include <string>
#include <unordered_map>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

class Light {

public:

	Light() = default;

	glm::vec3 position = glm::vec3(0.0f, 2.0f, -5.0f);

	glm::vec4 ambient = glm::vec4(1.0f);
	glm::vec4 diffuse = glm::vec4(1.0f);
	glm::vec4 specular = glm::vec4(1.0f);

};