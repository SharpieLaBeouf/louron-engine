#pragma once

#include <glad/glad.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

#include <map>

#include "Light.h"
#include "Camera.h"
#include "Mesh.h"
#include "../OpenGL/Material.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

namespace Louron {

	struct TagComponent {

		std::string Tag;

		TagComponent() = default;
		TagComponent(const TagComponent&) = default;
		TagComponent(const std::string& name) : Tag(name) { }
	};

	struct TransformComponent {

		glm::vec3 position = glm::vec3(0.0f);
		glm::vec3 rotation = glm::vec3(0.0f);
		glm::vec3 scale = glm::vec3(1.0f);

		TransformComponent() = default;
		TransformComponent(const TransformComponent&) = default;
		TransformComponent(const glm::vec3& translation) : position(translation) { }

		void Translate(const glm::vec3& vector) { position += vector; }
		void Rotate(const glm::vec3& vector) { rotation += vector; }
		void Scale(const glm::vec3& vector) { scale += vector; }

		operator glm::mat4() { return this->GetTransform(); }
		glm::mat4 GetTransform() const {
			return glm::translate(glm::mat4(1.0f), position)
				* glm::toMat4(glm::quat(glm::radians(rotation)))
				* glm::scale(glm::mat4(1.0f), scale);
		}
	};

	struct CameraComponent {

		Camera* Camera = nullptr;
		bool Primary = true;

		CameraComponent() = default;
		CameraComponent(const CameraComponent&) = default;

	};

}