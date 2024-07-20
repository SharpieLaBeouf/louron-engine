#pragma once

// Louron Core Headers

// C++ Standard Library Headers
#include <vector>

// External Vendor Library Headers
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace Louron {

	class Window;
	class InputManager;

	class Camera {

		friend class SceneSerializer;

	public:

		// Camera Options

		float NearDistance = 0.1f;
		float FarDistance = 1000.0f;

		float FOV = 60.0f;
		float MovementSpeed = 10.0f;
		float MovementYDamp = 0.65f;
		float MouseSensitivity = 0.1f;
		bool MouseToggledOff = true;

		Camera() = default;
		Camera(glm::vec3 position = glm::vec3(0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = -90.0f, float pitch = 0.0f);
		Camera(const Camera& other);
		~Camera() = default;

		void UpdateViewMatrix();
		glm::mat4 GetViewMatrix() const { return m_ViewMatrix; }

		void UpdateProjMatrix(); 
		glm::mat4 GetProjMatrix() const { return m_ProjMatrix; }

		glm::vec3 GetCameraDirection() const { return m_CameraFront; }

		void SetPosition(glm::vec3 pos) { m_CameraPos = pos; }
		glm::vec3 GetGlobalPosition() const { return m_CameraPos; }

		void SetPitch(float pitch) { m_Pitch = pitch; }
		float GetPitch() const { return m_Pitch; }

		void SetYaw(float yaw) { m_Yaw = yaw; }
		float GetYaw() const { return m_Yaw; }

		void ToggleMovement() { m_Movement = !m_Movement; }
		bool IsMovementEnabled() const { return m_Movement; }

		void Update(float deltaTime = 0.0f);

		void ProcessKeyboard(float deltaTime);
		void ProcessMouse(bool constrainPitch = true);

	private:

		Window& m_Window;
		InputManager& m_Input;

		glm::vec3 m_CameraPos;
		glm::vec3 m_CameraUp;
		glm::vec3 m_CameraRight;
		glm::vec3 m_CameraFront;
		glm::vec3 m_WorldUp;

		glm::mat4 m_ProjMatrix;
		glm::mat4 m_ViewMatrix;

		float m_Yaw;
		float m_Pitch;
		float m_LastMouseX = 0;
		float m_LastMouseY = 0;

		bool m_FirstMouse = true;
		bool m_Movement = true;

		void UpdateCameraVectors();
	};
}