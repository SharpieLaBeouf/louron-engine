#pragma once

// Louron Core Headers
#include "../../OpenGL/Shader.h"
#include "../../OpenGL/Texture.h"

#include "Components.h"

// C++ Standard Library Headers
#include <string>
#include <unordered_map>
#include <array>

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

	enum class ShadowTypeFlag {

		NoShadows	= 0,
		HardShadows = 1,
		SoftShadows = 2,

	};

	struct PointLightComponent : public Component {

		bool Active = true;

		glm::vec4 Colour = { 1.0f, 1.0f, 1.0f, 1.0f };

		float Radius = 10.0f;
		float Intensity = 1.0f;

		ShadowTypeFlag ShadowFlag = ShadowTypeFlag::NoShadows;

		PointLightComponent() = default;
		PointLightComponent(const PointLightComponent&) = default;

		void Serialize(YAML::Emitter& out);
		bool Deserialize(const YAML::Node data);
	};

	struct SpotLightComponent : public Component {

		bool Active = true;

		glm::vec4 Colour = { 1.0f, 1.0f, 1.0f, 1.0f };

		float Range = 20.0f;
		float Angle = 45.0f; // Full Angle of SpotLight Cone
		float Intensity = 1.0f;

		ShadowTypeFlag ShadowFlag = ShadowTypeFlag::NoShadows;

		SpotLightComponent() = default;
		SpotLightComponent(const SpotLightComponent&) = default;

		void Serialize(YAML::Emitter& out);
		bool Deserialize(const YAML::Node data);
	};

	struct DirectionalLightComponent : public Component {
		
		bool Active = true;

		glm::vec4 Colour = { 1.0f, 1.0f, 1.0f, 1.0f };

		float Intensity = 1.0f;

		ShadowTypeFlag ShadowFlag = ShadowTypeFlag::NoShadows;

		DirectionalLightComponent() = default;
		DirectionalLightComponent(const DirectionalLightComponent&) = default;

		void Serialize(YAML::Emitter& out);
		bool Deserialize(const YAML::Node data);
	};

	struct VisibleLightIndex {
		uint32_t index;

		VisibleLightIndex() = default;
		VisibleLightIndex(const VisibleLightIndex& other) = default;
		VisibleLightIndex(VisibleLightIndex&& other) = default;

		VisibleLightIndex& operator=(const VisibleLightIndex& other) = default;
		VisibleLightIndex& operator=(VisibleLightIndex&& other) = default;
	};
}