#include "Camera.h"

// Louron Core Headers
#include "../../Core/Engine.h"

// C++ Standard Library Headers


// External Vendor Library Headers


namespace Louron {

	Camera::Camera(glm::vec3 position, glm::vec3 up, float yaw, float pitch) : 
		m_Window(Engine::Get().GetWindow()), 
		m_Input(Engine::Get().GetInput())
	{
		
		m_CameraPos = position;
		m_CameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
		m_WorldUp = up;
		m_Yaw = yaw;
		m_Pitch = pitch;

		m_ProjMatrix = glm::perspective(glm::radians(FOV), (float)m_Window.GetWidth() / (float)m_Window.GetHeight(), NearDistance, FarDistance);

		UpdateCameraVectors();
	}

	Camera::Camera(const Camera& other) : m_Window(Engine::Get().GetWindow()), m_Input(Engine::Get().GetInput()) {

		m_CameraPos = other.m_CameraPos;
		m_CameraUp = other.m_CameraUp;
		m_CameraRight = other.m_CameraRight;
		m_CameraFront = other.m_CameraFront;
		m_WorldUp = other.m_WorldUp;

		m_Yaw = other.m_Yaw;
		m_Pitch = other.m_Pitch;
		m_LastMouseX = other.m_LastMouseX;
		m_LastMouseY = other.m_LastMouseY;

		m_FirstMouse = other.m_FirstMouse;
		m_Movement = other.m_Movement;

		m_ProjMatrix = other.m_ProjMatrix;
		m_ViewMatrix = other.m_ViewMatrix;

		NearDistance = other.NearDistance;
		FarDistance = other.FarDistance;

		FOV = other.FOV;
		MovementSpeed = other.MovementSpeed;
		MovementYDamp = other.MovementYDamp;
		MouseSensitivity = other.MouseSensitivity;
		MouseToggledOff = other.MouseToggledOff;
	}

	void Camera::UpdateViewMatrix() {
		m_ViewMatrix = glm::lookAt(m_CameraPos, m_CameraPos + m_CameraFront, m_CameraUp);
	}
	
	void Camera::UpdateProjMatrix() { m_ProjMatrix = glm::perspective(glm::radians(FOV), (float)m_Window.GetWidth() / (float)m_Window.GetHeight(), NearDistance, FarDistance); }
	void Camera::UpdateProjMatrix(const glm::ivec2& new_size) { m_ProjMatrix = glm::perspective(glm::radians(FOV), (float)new_size.x / (float)new_size.y, NearDistance, FarDistance); }

	void Camera::Update(float deltaTime) {

		if (m_Movement)
		{
			ProcessKeyboard(deltaTime);
			if (MouseToggledOff) 
				ProcessMouse(); 
			else if (!MouseToggledOff) {
				m_FirstMouse = true;
				UpdateCameraVectors();
			}

		}
		else UpdateCameraVectors();
	}

	void Camera::ProcessKeyboard(float deltaTime) {
		float modifier = (m_Input.GetKey(GLFW_KEY_LEFT_CONTROL)) ? 3.0f : 1.0f;
		float velocity = modifier * MovementSpeed * deltaTime;

		if (m_Input.GetKey(GLFW_KEY_W))
			m_CameraPos += velocity * m_CameraFront;
		if (m_Input.GetKey(GLFW_KEY_S))
			m_CameraPos -= velocity * m_CameraFront;

		if (m_Input.GetKey(GLFW_KEY_SPACE))
			m_CameraPos += velocity * m_WorldUp * MovementYDamp;
		if (m_Input.GetKey(GLFW_KEY_LEFT_SHIFT))
			m_CameraPos -= velocity * m_WorldUp * MovementYDamp;

		if (m_Input.GetKey(GLFW_KEY_D))
			m_CameraPos += velocity * m_CameraRight;
		if (m_Input.GetKey(GLFW_KEY_A))
			m_CameraPos -= velocity * m_CameraRight;
	}

	void Camera::ProcessMouse(bool constrainPitch) {
		float xpos = m_Input.GetMouseX(), ypos = m_Input.GetMouseY();

		if (m_FirstMouse) {
			m_LastMouseX = xpos;
			m_LastMouseY = ypos;
			m_FirstMouse = false;
		}

		float xoffset = xpos - m_LastMouseX;
		float yoffset = m_LastMouseY - ypos;

		m_LastMouseX = xpos;
		m_LastMouseY = ypos;

		xoffset *= MouseSensitivity;
		yoffset *= MouseSensitivity;

		m_Yaw += xoffset;
		m_Pitch += yoffset;

		if (constrainPitch) {
			if (m_Pitch > 89.0f)
				m_Pitch = 89.0f;
			if (m_Pitch < -89.0f)
				m_Pitch = -89.0f;
		}

		UpdateCameraVectors();
	}

	void Camera::LookAtGlobalPosition(const glm::vec3& global_position, float distance_from_position) {
		glm::vec3 direction = glm::normalize(global_position - m_CameraPos);
		m_Pitch = glm::degrees(glm::asin(direction.y));
		m_Yaw = glm::degrees(glm::atan(direction.z, direction.x));

		// Position the camera 10 units away from the target position
		m_CameraPos = global_position - direction * distance_from_position;

		UpdateCameraVectors();
	}

	void Camera::UpdateCameraVectors()
	{
		glm::vec3 front = glm::vec3();
		front.x = cos(glm::radians(m_Yaw)) * cos(glm::radians(m_Pitch));
		front.y = sin(glm::radians(m_Pitch));
		front.z = sin(glm::radians(m_Yaw)) * cos(glm::radians(m_Pitch));
		m_CameraFront = glm::normalize(front);

		m_CameraRight = glm::normalize(glm::cross(m_CameraFront, m_WorldUp));
		m_CameraUp = glm::normalize(glm::cross(m_CameraRight, m_CameraFront));
		
		UpdateViewMatrix();
	}
}