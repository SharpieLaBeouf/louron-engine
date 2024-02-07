#pragma once

#include "../OpenGL/Shader.h"
#include "../OpenGL/Texture.h"

#include <string>
#include <unordered_map>
#include <array>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

namespace Louron {

	struct alignas(16) PointLightComponent {

		glm::vec4 position = { 0.0f, 0.0f, 0.0f, 1.0f };

		glm::vec4 ambient = { 1.0f, 1.0f, 1.0f, 1.0f };
		glm::vec4 diffuse = { 1.0f, 1.0f, 1.0f, 1.0f };
		glm::vec4 specular = { 1.0f, 1.0f, 1.0f, 1.0f };

		struct LightProperties {
			GLfloat radius = 10.0f;
			GLfloat intensity = 1.0f;
			GLint active = true;
			GLint lastLight = false;
		} lightProperties;

		PointLightComponent() = default;
		PointLightComponent(const PointLightComponent&) = default;
	};

	struct alignas(16) SpotLightComponent {
		glm::vec4 position = glm::vec4(0.0f);
		glm::vec4 direction = glm::vec4(0.0f);

		glm::vec4 ambient = { 0.0f, 0.0f, 0.0f, 1.0f };
		glm::vec4 diffuse = { 1.0f, 1.0f, 1.0f, 1.0f };
		glm::vec4 specular = { 1.0f, 1.0f, 1.0f, 1.0f };

		struct LightProperties {
			GLfloat range = 50.0f;
			GLfloat angle = 45.0f;
			GLfloat intensity = 1.0f;
			GLint active = true;
		} lightProperties;

		GLint lastLight = false;

	public:
		
		// DO NOT USE - this is for SSBO padding and alignment purposes ONLY
		GLfloat m_Padding1 = 0.0f;
		GLfloat m_Padding2 = 0.0f;
		GLfloat m_Padding3 = 0.0f;

		SpotLightComponent() = default;
		SpotLightComponent(const SpotLightComponent&) = default;
	};

	struct alignas(16) DirectionalLightComponent {
	
	private:
		// This is private because the Transform Component holds the direction
		// we just need this here for the SSBO data parsing and alignment.
		glm::vec4 direction = glm::vec4(0.0f);
	
	public:
		
		glm::vec4 ambient = glm::vec4(0.2f);
		glm::vec4 diffuse = glm::vec4(1.0f);
		glm::vec4 specular = glm::vec4(0.5f);
		
		GLint lastLight = false;

	private:

		// DO NOT USE - this is for SSBO padding and alignment purposes ONLY
		GLfloat m_Padding1 = 0.0f;
		GLfloat m_Padding2 = 0.0f;
		GLfloat m_Padding3 = 0.0f;

	public:

		friend class ForwardPlusPipeline;
	};

	struct VisibleLightIndex {
		GLint index;
	};
}