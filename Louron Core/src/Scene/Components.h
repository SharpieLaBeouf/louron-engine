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

	struct Transform {

	private:
		glm::vec3 m_Position = glm::vec3(0.0f);
		glm::vec3 m_Rotation = glm::vec3(0.0f);
		glm::vec3 m_Scale = glm::vec3(1.0f);
		glm::mat4 m_Transform = glm::mat4(1.0f);

	public:

		Transform() { UpdateTransform(); }
		Transform(const Transform&) = default;
		Transform(const glm::vec3& translation) : m_Position(translation) { UpdateTransform(); }

		/// <summary>
		/// Set the position to a fixed value.
		/// </summary>
		/// <param name="newScale">This will be the new fixed position.</param>
		void SetPosition(const glm::vec3& newPosition) {
			m_Position = newPosition;
			UpdateTransform();
		}

		void SetPositionX(const float& newXPosition) {
			m_Position.x = newXPosition;
			UpdateTransform();
		}

		void SetPositionY(const float& newYPosition) {
			m_Position.y = newYPosition;
			UpdateTransform();
		}

		void SetPositionZ(const float& newZPosition) {
			m_Position.z = newZPosition;
			UpdateTransform();
		}

		/// <summary>
		/// Set the rotation to a fixed value.
		/// </summary>
		/// <param name="newScale">This will be the new fixed rotation.</param>
		void SetRotation(const glm::vec3& newRotation) {
			m_Rotation = newRotation;
			UpdateTransform();
		}

		void SetRotationX(const float& newXRotation) {
			m_Rotation.x = newXRotation;
			UpdateTransform();
		}

		void SetRotationY(const float& newYRotation) {
			m_Rotation.y = newYRotation;
			UpdateTransform();
		}

		void SetRotationZ(const float& newZRotation) {
			m_Rotation.z = newZRotation;
			UpdateTransform();
		}
		/// <summary>
		/// Set the scale to a fixed value.
		/// </summary>
		/// <param name="newScale">This will be the new fixed scale.</param>
		void SetScale(const glm::vec3& newScale) {
			m_Scale = newScale;
			UpdateTransform();
		}

		void SetScaleX(const float& newXScale) {
			m_Scale.x = newXScale;
			UpdateTransform();
		}

		void SetScaleY(const float& newYScale) {
			m_Scale.y = newYScale;
			UpdateTransform();
		}

		void SetScaleZ(const float& newZScale) {
			m_Scale.z = newZScale;
			UpdateTransform();
		}

		/// <summary>
		/// Apply a Translation to the Transform.
		/// </summary>
		/// <param name="vector">This will be added to the current position.</param>
		void Translate(const glm::vec3& vector) { 
			m_Position += vector;
			UpdateTransform();
		}

		/// <summary>
		/// Apply a Rotation to the Transform.
		/// </summary>
		/// <param name="vector">This will be added to the current rotation.</param>
		void Rotate(const glm::vec3& vector) { 
			m_Rotation += vector;
			UpdateTransform();
		}

		/// <summary>
		/// Apply a Scale to the Transform.
		/// </summary>
		/// <param name="vector">This will be added to the current scale.</param>
		void Scale(const glm::vec3& vector) { 
			m_Scale += vector;
			UpdateTransform();
		}

		const glm::vec3& GetPosition() { return m_Position; }
		const glm::vec3& GetRotation() { return m_Rotation; }
		const glm::vec3& GetScale() { return m_Scale; }

		const glm::mat4& GetTransform() { return m_Transform; }

		operator const glm::mat4()& { return m_Transform; }

	private:
		void UpdateTransform() {
			m_Transform = glm::translate(glm::mat4(1.0f), m_Position)
						* glm::toMat4(glm::quat(glm::radians(m_Rotation)))
						* glm::scale(glm::mat4(1.0f), m_Scale);
		}

	};

	struct CameraComponent {

		Camera* Camera = nullptr;
		bool Primary = true;

		CameraComponent() = default;
		CameraComponent(const CameraComponent&) = default;

	};

}