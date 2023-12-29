#pragma once

#include "../OpenGL/Shader.h"
#include "../OpenGL/Texture.h"

#include <string>
#include <unordered_map>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

namespace Louron {

	// TODO: Create Point Light Struct
	class Light {

	public:

		Light() = default;

		glm::vec3 position = glm::vec3(0.0f);

		glm::vec4 ambient = glm::vec4(1.0f);
		glm::vec4 diffuse = glm::vec4(1.0f);
		glm::vec4 specular = glm::vec4(1.0f);

	};
}