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

	// TODO: Remove Old Light Struct
	struct OldLight {
		glm::vec3 position = glm::vec3(0.0f);

		glm::vec4 ambient = glm::vec4(1.0f);
		glm::vec4 diffuse = glm::vec4(1.0f);
		glm::vec4 specular = glm::vec4(1.0f);

		OldLight() = default;
	};

	struct PointLightComponent {

		glm::vec4 position = { 0.0f, 0.0f, 0.0f, 1.0f };

		glm::vec4 ambient = { 1.0f, 1.0f, 1.0f, 1.0f };
		glm::vec4 diffuse = { 1.0f, 1.0f, 1.0f, 1.0f };
		glm::vec4 specular = { 1.0f, 1.0f, 1.0f, 1.0f };

		float constant = 1.0f;
		float linear = 0.09f;
		float quadratic = 0.032f;

		bool lastLight = false;

		PointLightComponent() = default;
		PointLightComponent(const PointLightComponent&) = default;
	};

	struct SpotLightComponent {
		glm::vec4 position = glm::vec4(0.0f);
		glm::vec4 direction = glm::vec4(0.0f);

		glm::vec4 ambient = { 0.0f, 0.0f, 0.0f, 1.0f };
		glm::vec4 diffuse = { 1.0f, 1.0f, 1.0f, 1.0f };
		glm::vec4 specular = { 1.0f, 1.0f, 1.0f, 1.0f };

		float cutOff = glm::cos(glm::radians(12.5f));
		float outerCutOff = glm::cos(glm::radians(15.0f));

		float constant = 1.0f;
		float linear = 0.09f;
		float quadratic = 0.032f;

		bool lastLight = false;

	private:

		// DO NOT USE - this is for SSBO alignment purposes ONLY
		std::array<float, 2> temp{ 0.0f };

	public:

		SpotLightComponent() = default;
		SpotLightComponent(const SpotLightComponent&) = default;
	};

	struct DirectionalLightComponent {
	
	private:
		// This is private because the Transform Component holds the direction
		// we just need this here for the SSBO data parsing and alignment.
		glm::vec4 direction = glm::vec4(0.0f);
	
	public:
		
		glm::vec4 ambient = glm::vec4(0.2f);
		glm::vec4 diffuse = glm::vec4(1.0f);
		glm::vec4 specular = glm::vec4(0.5f);
		
		bool lastLight = false;

	private:

		// DO NOT USE - this is for SSBO alignment purposes ONLY
		std::array<float, 3> temp{ 0.0f };

	public:

		friend class Scene;
	};

	struct VisibleLightIndex {
		int index;
	};
}