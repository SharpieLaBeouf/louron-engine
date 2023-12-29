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
	struct OldLight {
		glm::vec3 position = glm::vec3(0.0f);

		glm::vec4 ambient = glm::vec4(1.0f);
		glm::vec4 diffuse = glm::vec4(1.0f);
		glm::vec4 specular = glm::vec4(1.0f);

		OldLight() = default;
	};

	struct PointLightComponent {

		glm::vec3 position;

		float Constant;
		float Linear;
		float Quadratic;
		
		glm::vec3 ambient;
		glm::vec3 diffuse;
		glm::vec3 specular;

		PointLightComponent() = default;
		PointLightComponent(const PointLightComponent&) = default;
	};

	struct SpotLight {
		glm::vec3 position;
		glm::vec3 direction;

		float cutOff;
		float outerCutOff;

		float constant;
		float linear;
		float quadratic;

		glm::vec3 ambient;
		glm::vec3 diffuse;
		glm::vec3 specular;

		SpotLight() = default;
		SpotLight(const SpotLight&) = default;
	};
}